#include "serial.h"
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QFile>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>
#include <QMessageBox>

static const quint8  FRAME_HDR1      = 0xAA;
static const quint8  FRAME_HDR2      = 0x55;

static const quint16 CMD_START       = 0x0001;   // PC→MCU 开始，数据=4字节总长
static const quint16 CMD_DATA        = 0x0002;   // PC→MCU 数据，保留=偏移
static const quint16 CMD_END         = 0x0003;   // PC→MCU 结束
static const quint16 CMD_ACK         = 0x0004;   // MCU→PC 成功
static const quint16 CMD_NAK         = 0x0005;   // MCU→PC 失败，数据=位图

static const int FRAME_DATA_LEN = 256;           // 每帧数据区长度
static const int MAX_NAK_RETRY  = 3;             // NAK/超时最多重发次数
static const int ACK_TIMEOUT_MS = 2000;          // 等应答超时(ms)

Serial::Serial(QComboBox *port, QComboBox *baud,
               QLineEdit *path,
               QPushButton *choseFileBtn, QPushButton *findPortBtn, QPushButton *openSerialBtn, QPushButton *sendFileBtn,
               QPushButton *sendBtn, QPushButton *clearBtn,
               QTextEdit *recv, QTextEdit *sendText,
               QObject *parent)
    : QObject(parent)
    , m_port(port)
    , m_baud(baud)
    , m_path(path)
    , m_choseFileBtn(choseFileBtn)
    , m_findPortBtn(findPortBtn)
    , m_openSerialBtn(openSerialBtn)
    , m_sendFileBtn(sendFileBtn)
    , m_sendBtn(sendBtn)
    , m_clearBtn(clearBtn)
    , m_recv(recv)
    , m_sendText(sendText)
{
    m_serial = new QSerialPort(this);

    connect(m_choseFileBtn, &QPushButton::clicked, this, &Serial::onChoseFile);
    connect(m_findPortBtn,  &QPushButton::clicked, this, &Serial::onFindPort);
    connect(m_openSerialBtn,&QPushButton::clicked, this, &Serial::onOpenSerial);
    connect(m_sendFileBtn,  &QPushButton::clicked, this, &Serial::onSendFile);
    connect(m_sendBtn,      &QPushButton::clicked, this, &Serial::onSend);
    connect(m_clearBtn,     &QPushButton::clicked, this, &Serial::onClear);

    onFindPort();
}


void Serial::onChoseFile()
{
    QString path = QFileDialog::getOpenFileName();
    if(path.isEmpty()){
        QMessageBox::warning(nullptr,"打开文件","选择文件路径不能为空");
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        m_recv->append("打开文件失败");
        return;
    }
    m_fw = file.readAll();
    file.close();
    m_recv->append(QString("已加载固件 %1，共 %2 字节").arg(path).arg(m_fw.size()));

    m_path->setText(path);

}


void Serial::onFindPort()
{
    m_port->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &info : ports){
        m_port->addItem(info.portName());
    }
}


void Serial::onOpenSerial()
{
    if(m_openSerialBtn->text() == "打开串口"){
        m_serial->setPortName(m_port->currentText());
        m_serial->setBaudRate(m_baud->currentText().toInt());
        m_serial->open(QIODevice::ReadWrite);
        if(m_serial->isOpen()){
            qDebug() << "串口开启成功";
            m_openSerialBtn->setText("关闭串口");
            connect(m_serial,&QSerialPort::readyRead,this,&Serial::onReadyRead);
        }else{
            qDebug() << "串口开启失败";
        }
    }else{
        m_serial->close();
        if(m_serial->isOpen()){
            qDebug() << "串口关闭失败";
        }else{
            qDebug() << "串口关闭成功";
            m_openSerialBtn->setText("打开串口");
        }
    }

}


void Serial::onSendFile()
{
    if (m_fw.isEmpty()) { qDebug()<<"请先选择固件文件"; return; }
    if (!m_serial->isOpen()) { qDebug()<<"请先打开串口"; return; }

    m_retry = 0;
    m_upgrading = true;
    sendAllFrames();

    m_recv->append(QString("已发送 %1 字节，等待 MCU 应答...").arg(m_fw.size()));
    m_timeout.start(ACK_TIMEOUT_MS);

}

void Serial::onSend()
{
    QByteArray data = m_sendText->toPlainText().toUtf8();
    m_serial->write(data);
}

void Serial::onClear()
{
    m_recv->clear();
}


void Serial::onReadyRead()
{
    QByteArray rx_data = m_serial->readAll();
    m_recv->append(QString::fromLocal8Bit(rx_data));

    m_rxBuf.append(rx_data);

    while (true) {
        if (m_rxBuf.size() < 6) return;   // 包头2+命令2+长度2

        // 找包头 0xAA 0x55
        int idx = -1;
        for (int i = 0; i + 1 < m_rxBuf.size(); ++i) {
            if (static_cast<quint8>(m_rxBuf[i])   == FRAME_HDR1 &&
                static_cast<quint8>(m_rxBuf[i+1]) == FRAME_HDR2) {
                idx = i;
                break;
            }
        }
        if (idx < 0) { m_rxBuf.clear(); return; }   // 没包头，全丢
        if (idx > 0) m_rxBuf.remove(0, idx);        // 丢包头前垃圾

        if (m_rxBuf.size() < 6) return;             // 还没拿到长度

        quint16 len = static_cast<quint8>(m_rxBuf[4]) |
                      (static_cast<quint8>(m_rxBuf[5]) << 8);
        if (len > FRAME_DATA_LEN) {                 // 非法长度，丢1字节重找
            m_rxBuf.remove(0, 1);
            continue;
        }
        int total = 12 + len;
        if (m_rxBuf.size() < total) return;         // 半包，等下一批

        QByteArray frame = m_rxBuf.left(total);
        m_rxBuf.remove(0, total);

        // CRC 校验
        quint16 crcRecv = static_cast<quint8>(frame[10+len]) |
                          (static_cast<quint8>(frame[11+len]) << 8);
        if (crc16Modbus(frame.mid(2, 8+len)) != crcRecv)
            continue;                                // CRC 错，丢

        quint16 cmd = static_cast<quint8>(frame[2]) |
                      (static_cast<quint8>(frame[3]) << 8);
        QByteArray data = frame.mid(6, len);

        if (cmd == CMD_ACK)      handleAck();
        else if (cmd == CMD_NAK) handleNak(data);
        // 其它命令本端不收，忽略
    }
}

void Serial::onTimeout()
{
    if (!m_upgrading) return;              // 不在升级流程，忽略（定时器可能残留）
    if (++m_retry > MAX_NAK_RETRY) {
        m_upgrading = false;
        qDebug()<<"升级失败：超时未收到应答";
        return;
    }
    qDebug()<<"超时，第 "<<m_retry<<" 次整包重发...";
    sendAllFrames();
    m_timeout.start(ACK_TIMEOUT_MS);
}



QByteArray Serial::buildFrame(quint16 cmd, const QByteArray &data, quint32 offset)
{
    quint16 len = (quint16)data.size();
    QByteArray f;
    f.append((char)FRAME_HDR1);
    f.append((char)FRAME_HDR2);
    f.append((char)(cmd));
    f.append((char)(cmd >> 8));
    f.append((char)(len));
    f.append((char)(len >> 8));
    f.append(data);
    f.append((char)(offset));
    f.append((char)(offset>>8));
    f.append((char)(offset>>16));
    f.append((char)(offset>>24));
    quint16 crc = crc16Modbus(f.mid(2));
    f.append((char)(crc));
    f.append((char)(crc>>8));
    return f;

}

quint16 Serial::crc16Modbus(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (char c : data) {
        crc ^= static_cast<quint8>(c);        // 关键：先转无符号
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

void Serial::sendAllFrames()
{
    QByteArray startData;
    int total = m_fw.size();
    quint32 t = (quint32)total;
    startData.append((char)(t));
    startData.append((char)(t >> 8));
    startData.append((char)(t >> 16));
    startData.append((char)(t >> 24));

    m_serial->write(buildFrame(CMD_START,startData,0));

    for(int offset = 0;offset<total;offset+=FRAME_DATA_LEN ){
        int len = qMin(FRAME_DATA_LEN, total - offset);
        m_serial->write(buildFrame(CMD_DATA,m_fw.mid(offset, len),offset));
    }
    m_serial->write(buildFrame(CMD_END, QByteArray(), 0));
}

void Serial::handleAck()
{
    m_timeout.stop();
    m_upgrading = false;
    qDebug()<<"升级成功";
}

void Serial::handleNak(const QByteArray &bitmap)
{
    m_timeout.stop();
    if (++m_retry > MAX_NAK_RETRY) {
        m_upgrading = false;
        qDebug()<<"升级失败：重发次数过多";
        return;
    }

    qDebug()<<"收到 NAK，重发失败帧,第 "<<m_retry<<" 次";

    int total = m_fw.size();

    for (int i = 0; i < bitmap.size(); ++i) {
        quint8 bits = static_cast<quint8>(bitmap[i]);
        for (int b = 0; b < 8; ++b) {
            if (bits & (1 << b)) continue;          // 该帧成功，跳过
            int off = (i * 8 + b) * FRAME_DATA_LEN;
            if (off >= total) break;
            int len = qMin(FRAME_DATA_LEN, total - off);
            m_serial->write(buildFrame(CMD_DATA, m_fw.mid(off, len),
                                       static_cast<quint32>(off)));
        }
    }

    m_serial->write(buildFrame(CMD_END, QByteArray(), 0));  // 补发 END
    m_timeout.start(ACK_TIMEOUT_MS);
}

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
    QFile file(m_path);

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
    m_recv->append(rx_data);

}

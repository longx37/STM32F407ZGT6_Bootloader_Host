#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <QByteArray>

class QComboBox;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QSerialPort;

class Serial : public QObject
{
    Q_OBJECT
public:
    explicit Serial(QComboBox *port, QComboBox *baud,
                    QLineEdit *path,
                    QPushButton *choseFileBtn, QPushButton *findPortBtn, QPushButton *openSerialBtn, QPushButton *sendFileBtn,
                    QPushButton *sendBtn, QPushButton *clearBtn,
                    QTextEdit *recv, QTextEdit *sendText,
                    QObject *parent = nullptr);

private slots:
    void onChoseFile();     // 选择 .bin 固件
    void onFindPort();      // 枚举串口
    void onOpenSerial();    // 打开 / 关闭串口
    void onSendFile();      // 发送固件
    void onSend();          // 发送文本
    void onClear();         // 清空接收区
    void onReadyRead();     // 串口收到数据

private:
    QComboBox   *m_port;
    QComboBox   *m_baud;
    QLineEdit   *m_path;

    QPushButton *m_choseFileBtn;
    QPushButton *m_findPortBtn;
    QPushButton *m_openSerialBtn;
    QPushButton *m_sendFileBtn;
    QPushButton *m_sendBtn;
    QPushButton *m_clearBtn;

    QTextEdit   *m_recv;
    QTextEdit   *m_sendText;

    QSerialPort *m_serial = nullptr;
    QByteArray   m_fw;
};

#endif // SERIAL_H

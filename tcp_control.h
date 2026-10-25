#ifndef TCP_CONTROL_H
#define TCP_CONTROL_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class QComboBox;
class QLineEdit;
class QPushButton;
class QTextEdit;

class TCP_Control : public QObject
{
    Q_OBJECT
public:
    explicit TCP_Control(QComboBox *role,
                         QLineEdit *sip, QLineEdit *sport,
                         QLineEdit *dip, QLineEdit *dport,
                         QPushButton *start, QPushButton *stop,
                         QPushButton *send, QPushButton *clear,
                         QTextEdit *recv, QTextEdit *sendText,
                         QObject *parent = nullptr);

private:
    QTcpServer *m_server =nullptr;
    QTcpSocket *m_socket = nullptr;


    QComboBox   *m_role;
    QLineEdit   *m_sip;
    QLineEdit   *m_sport;
    QLineEdit   *m_dip;
    QLineEdit   *m_dport;
    QPushButton *m_startBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_sendBtn;
    QPushButton *m_clearBtn;
    QTextEdit   *m_recv;
    QTextEdit   *m_sendText;

private slots:
    void onStart();
    void onReadyRead();
    void onStop();
    void onSend();
    void onClear();

};

#endif // TCP_CONTROL_H

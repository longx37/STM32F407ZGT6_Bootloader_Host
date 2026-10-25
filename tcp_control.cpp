#include "tcp_control.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QHostAddress>

TCP_Control::TCP_Control(QComboBox *role,
                             QLineEdit *sip, QLineEdit *sport,
                             QLineEdit *dip, QLineEdit *dport,
                             QPushButton *start, QPushButton *stop,
                             QPushButton *send, QPushButton *clear,
                             QTextEdit *recv, QTextEdit *sendText,
                             QObject *parent)
    : QObject(parent)
    , m_role(role)
    , m_sip(sip), m_sport(sport)
    , m_dip(dip), m_dport(dport)
    , m_startBtn(start), m_stopBtn(stop)
    , m_sendBtn(send), m_clearBtn(clear)
    , m_recv(recv), m_sendText(sendText)
{


    connect(m_startBtn,&QPushButton::clicked,this,&TCP_Control::onStart);
    connect(m_stopBtn,&QPushButton::clicked,this,&TCP_Control::onStop);
    connect(m_sendBtn,  &QPushButton::clicked, this, &TCP_Control::onSend);
    connect(m_clearBtn, &QPushButton::clicked, this, &TCP_Control::onClear);
}

void TCP_Control::onStart()
{
    if(m_role->currentText() == "client"){
        m_socket = new QTcpSocket(this);
        m_socket->connectToHost(m_dip->text(),m_dport->text().toUShort());
        connect(m_socket,&QTcpSocket::connected,this,[=]{
            qDebug() << "连接成功";
            m_startBtn->setDisabled(true);
        });
        connect(m_socket,&QTcpSocket::readyRead,this,&TCP_Control::onReadyRead);
        connect(m_socket,&QTcpSocket::disconnected,this,[=]{
            m_socket->deleteLater();
            m_socket = nullptr;
            qDebug()<<"已断开连接";
        });
    }
}

void TCP_Control::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    m_recv->append(data);

}

void TCP_Control::onStop()
{
    if (m_socket) m_socket->abort();
    m_startBtn->setDisabled(false);
}

void TCP_Control::onSend()
{
    QByteArray data = m_sendText->toPlainText().toUtf8();
    m_socket->write(data);
}

void TCP_Control::onClear()
{
    m_recv->clear();
}


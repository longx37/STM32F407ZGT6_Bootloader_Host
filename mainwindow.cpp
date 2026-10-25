#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

class TCP_Control;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->wifi_receive_text->setReadOnly(true);
    ui->eth_receive_text->setReadOnly(true);
    ui->wifi_sip->setText("192.168.4.2");
    ui->wifi_sport->setText("9999");
    ui->wifi_dip->setText("192.168.4.1");
    ui->wifi_dport->setText("8080");

    ui->eth_sip->setText("192.168.137.1");
    ui->eth_sport->setText("9999");
    ui->eth_dip->setText("192.168.137.137");
    ui->eth_dport->setText("9999");

    m_wifi = new TCP_Control(ui->wifi_role,
                               ui->wifi_sip, ui->wifi_sport,
                               ui->wifi_dip, ui->wifi_dport,
                               ui->wifi_start, ui->wifi_stop,
                               ui->wifi_send, ui->wifi_clear,
                               ui->wifi_receive_text, ui->wifi_send_text,
                               this);

    m_eth = new TCP_Control(ui->eth_role,
                              ui->eth_sip, ui->eth_sport,
                              ui->eth_dip, ui->eth_dport,
                              ui->eth_start, ui->eth_stop,
                              ui->eth_send, ui->eth_clear,
                              ui->eth_receive_text, ui->eth_send_text,
                              this);

    m_serial = new Serial(ui->serial_port, ui->serial_baud,
                        ui->file_path,
                        ui->chose_file, ui->find_port,ui->serial_open, ui->sendfile,
                        ui->serial_send, ui->serial_clear,
                        ui->serial_receive_text, ui->serial_send_text,
                        this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_wifi_role_currentTextChanged(const QString &arg1)
{
    if(arg1 == "server"){
        ui->wifi_sip->setText("192.168.2.137");
        ui->wifi_sip->setReadOnly(true);
        ui->wifi_sport->setText("9999");
        ui->wifi_sport->setReadOnly(true);
        ui->wifi_sport->setStyleSheet("color:gray");
        ui->wifi_sip->setStyleSheet("color:gray");
    }else if(arg1 == "client"){
        ui->wifi_sip->setReadOnly(false);
        ui->wifi_sport->setReadOnly(false);
        ui->wifi_sport->setStyleSheet("color:black");
        ui->wifi_sip->setStyleSheet("color:black");
    }
}


void MainWindow::on_eth_role_currentTextChanged(const QString &arg1)
{
    if(arg1 == "server"){
        ui->eth_sip->setText("192.168.137.137");
        ui->eth_sip->setReadOnly(true);
        ui->eth_sport->setText("9999");
        ui->eth_sport->setReadOnly(true);
        ui->eth_sport->setStyleSheet("color:gray");
        ui->eth_sip->setStyleSheet("color:gray");
    }else if(arg1 == "client"){
        ui->eth_sip->setReadOnly(false);
        ui->eth_sport->setReadOnly(false);
        ui->eth_sport->setStyleSheet("color:black");
        ui->eth_sip->setStyleSheet("color:black");
    }
}





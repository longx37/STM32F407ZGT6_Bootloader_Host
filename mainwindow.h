#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tcp_control.h"
#include "serial.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_wifi_role_currentTextChanged(const QString &arg1);

    void on_eth_role_currentTextChanged(const QString &arg1);


private:
    Ui::MainWindow *ui;
    TCP_Control *m_wifi = nullptr;
    TCP_Control *m_eth = nullptr;
    Serial *m_serial = nullptr;

};
#endif // MAINWINDOW_H

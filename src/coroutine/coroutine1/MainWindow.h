#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkDatagram>
#include <QUdpSocket>
#include "coroutine.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void onRecv();
    void onProcess1(const QNetworkDatagram &datagram);
    void onProcess2(const QNetworkDatagram &datagram);
    void on_btn1_clicked();

    void on_btn2_clicked();

signals:
    void sigRecv(const QNetworkDatagram &datagram);

private:
    void process1(schedule *schDl, void *ud);
    void process2(schedule *schDl, void *ud);
    Ui::MainWindow *ui;
    QUdpSocket *socket_;
    schedule *schedule_;
    bool res1_;
    bool res2_;
};
#endif // MAINWINDOW_H

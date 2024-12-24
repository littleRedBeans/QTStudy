#include "MainWindow.h"
#include <QDebug>
#include <QEventLoop>
#include <QHostAddress>
#include "ui_MainWindow.h"
#include <functional>
using std::placeholders::_1;
using std::placeholders::_2;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket_(new QUdpSocket)
    , schedule_(coroutine_open())

{
    ui->setupUi(this);
    qDebug() << "mian coID:" << coroutine_running(schedule_);
    int co1 = coroutine_new(schedule_, std::bind(&MainWindow::process1, this, _1, _2), nullptr);
    int co2 = coroutine_new(schedule_, std::bind(&MainWindow::process2, this, _1, _2), nullptr);
    qDebug() << "co1:" << co1 << "co2:" << co2;
    socket_->bind(QHostAddress("0.0.0.0"), 10000);
    connect(socket_, &QUdpSocket::readyRead, this, &MainWindow::onRecv);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onRecv()
{
    while (socket_->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket_->receiveDatagram();
        //        processTheDatagram(datagram);
    }
}

void MainWindow::onProcess1(const QNetworkDatagram &datagram) {}

void MainWindow::onProcess2(const QNetworkDatagram &datagram) {}

void MainWindow::on_btn1_clicked()
{
    qDebug() << "on_btn1_clicked current coID:" << coroutine_running(schedule_);
    ui->btn1->setDisabled(true);
    coroutine_resume(schedule_, 0);
    ui->btn1->setDisabled(false);
    qDebug() << "on_btn1_clicked current coID:" << coroutine_running(schedule_);
}

void MainWindow::process1(schedule *schDl, void *ud)
{
    qDebug() << "process1 current coID:" << coroutine_running(schDl);
    res1_ = false;
    socket_->writeDatagram(QByteArray::fromHex("11 11 11"), QHostAddress("192.168.1.6"), 12000);
    coroutine_yield(schDl);
    if (res1_) {
        qDebug() << "process1 successed";
    } else {
        qDebug() << "process1 fail";
    }
    ((QEventLoop *) ud)->quit();
}

void MainWindow::process2(schedule *schDl, void *ud)
{
    qDebug() << "process2 current coID:" << coroutine_running(schDl);
    res2_ = false;
    socket_->writeDatagram(QByteArray::fromHex("22 22 22"), QHostAddress("192.168.1.6"), 12000);
    if (res2_) {
        qDebug() << "process2 successed";
    } else {
        qDebug() << "process2 fail";
    }
}

void MainWindow::on_btn2_clicked()
{
    qDebug() << "on_btn2_clicked current coID:" << coroutine_running(schedule_);
    ui->btn2->setDisabled(true);
    coroutine_resume(schedule_, 1);
    ui->btn2->setDisabled(false);
    qDebug() << "on_btn2_clicked current coID:" << coroutine_running(schedule_);
}

#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include "UdpServer.h"
#include <unistd.h>
using namespace shuimo;
int main(int argc, char *argv[])
{
    qDebug() << "test QUdpSocket main thread id:" << getpid();
    QCoreApplication a(argc, argv);

    UdpServer server("192.168.1.3", 9001);
    auto recvCb = [](const QString &addr, quint16 port, const QByteArray &data) {
        qDebug() << QString("recv %1:%2 %3bytes").arg(addr).arg(port).arg(data.size());
    };
    QObject::connect(&server, &UdpServer::dataReceived, recvCb);
    QThread thread;
    server.moveToThread(&thread);
    thread.start();

    QTimer::singleShot(0, &server, &UdpServer::initSocket);
    return a.exec();
}

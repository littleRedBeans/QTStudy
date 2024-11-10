#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include "UvUdpSocket.h"
#include <unistd.h>
using namespace shuimo;

int main(int argc, char *argv[])
{
    qDebug() << "main thread tid:" << getpid();
    QCoreApplication a(argc, argv);

    //create socket
    UvUdpSocket socket("192.168.1.3", 9001);
    auto recvCb = [](const QString &addr, quint16 port, const QByteArray &data) {
        qDebug() << QString("recv %1:%2 %3bytes").arg(addr).arg(port).arg(data.size());
    };
    QObject::connect(&socket, &UvUdpSocket::dataReceived, recvCb);

    //move to child thread
    QThread thread;
    socket.moveToThread(&thread);
    thread.start();

    //excute initSocket() in child thread
    QTimer::singleShot(0, &socket, &UvUdpSocket::initSocket);

    //excute this code will crash
    //QString message("hello world");
    //socket.sendData(message.toLocal8Bit(), "192.168.1.6", 8080);

    return a.exec();
}

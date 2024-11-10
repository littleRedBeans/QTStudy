#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QUdpSocket>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QUdpSocket socket;
    socket.bind(QHostAddress("192.168.1.3"), 9002);
    QByteArray data(1024, 0);
    for (int i = 0; i < 1024 * 100; ++i) {
        socket.writeDatagram(data, QHostAddress("192.168.1.3"), 9001);
    }
    qDebug() << QString("100 MiB bytes have been sent");
    data[0] = 0x01;
    socket.writeDatagram(data, QHostAddress("192.168.1.3"), 9001);
    return a.exec();
}

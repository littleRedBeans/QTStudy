#include "UdpServer.h"
#include <unistd.h>
using namespace shuimo;

quint64 g_currRecvSize = 0;
UdpServer::UdpServer(const QString &ip, quint16 port, QObject *parent)
    : QObject(parent)
    , ip_(ip)
    , port_(port)
    , socket_(this)
{}

void UdpServer::initSocket()
{
    socket_.setReadBufferSize(65536);
    socket_.bind(QHostAddress(ip_), port_);
    connect(&socket_, &QUdpSocket::readyRead, this, &UdpServer::readPendingDatagrams);
}

void UdpServer::readPendingDatagrams()
{
    //    qDebug() << "UdpServer::readPendingDatagrams() thread id:" << gettid();
    while (socket_.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket_.pendingDatagramSize());
        QHostAddress senderAddress;
        quint16 senderPort;
        qint64 recv = socket_.readDatagram(datagram.data(),
                                           datagram.size(),
                                           &senderAddress,
                                           &senderPort);
        ///////////////only for test performance///////////////////////
        g_currRecvSize += recv;
        if (datagram[0] == 0x01) {
            qDebug() << QString("recv %1 MiB data").arg(g_currRecvSize / 1024.0 / 1024.0);
        }
        //////////////////////////////////////////////////////////////
        emit dataReceived(senderAddress.toString(), senderPort, datagram);
    }
}

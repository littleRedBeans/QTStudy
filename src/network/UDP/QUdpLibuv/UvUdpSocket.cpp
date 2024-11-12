#include "UvUdpSocket.h"
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <EventDispatcherLibUv.h>
#include <unistd.h>
using namespace shuimo;
using namespace std;
using namespace std::placeholders;

quint64 g_currRecvSize = 0;
UvUdpSocket::UvUdpSocket(const QString &ip, quint16 port, QObject *parent)
    : QObject(parent)
    , ip_(ip)
    , port_(port)
    , start_(false)
{}

UvUdpSocket::~UvUdpSocket()
{
    closeSocket();
    uv_loop_close(loop_);
}

void UvUdpSocket::initSocket()
{
    assert(false == start_);
    start_ = true;
    //create uv_loop_t uv_udp_t
    EventDispatcherLibUv *dispatcher = static_cast<EventDispatcherLibUv *>(
        QAbstractEventDispatcher::instance());
    loop_ = dispatcher->uvLoop();
    udp_socket_ = make_shared<uv_udp_t>();

    //set context UvUdpSocket pointer to udp_socket_
    uv_handle_set_data((uv_handle_t *) udp_socket_.get(), this);

    //bind to Ip:port
    uv_udp_init(loop_, udp_socket_.get());
    struct sockaddr_in addr;
    uv_ip4_addr(ip_.toStdString().c_str(), port_, &addr);

    int r = uv_udp_bind(udp_socket_.get(), (const struct sockaddr *) &addr, 0);
    if (r) {
        qDebug() << "Failed to bind UDP socket: " << uv_strerror(r);
    }

    r = uv_udp_recv_start(udp_socket_.get(), allocBuffer, onUdpReadStatic);
    if (r) {
        qDebug() << "Failed to start UDP socket: " << uv_strerror(r);
    }
}

void UvUdpSocket::sendDataPrivate(const QByteArray &data, const QString &address, quint16 port)
{
    struct sockaddr_in addr;
    uv_ip4_addr(address.toStdString().c_str(), port, &addr);

    auto *context = new SendContext;
    context->buffer = new char[data.size()];
    memcpy(context->buffer, data.constData(), data.size());

    context->uv_buf.base = context->buffer;
    context->uv_buf.len = data.size();

    // create uv_udp_send_t
    auto *send_req = new uv_udp_send_t;
    // save context into send_req
    send_req->data = context;

    int r = uv_udp_send(send_req,
                        udp_socket_.get(),
                        &context->uv_buf,
                        1,
                        (const struct sockaddr *) &addr,
                        onSendComplete);
    if (r) {
        qDebug() << "Failed to send UDP data: " << uv_strerror(r);
        // delete context
        delete[] context->buffer;
        delete context;
        delete send_req;
    }
}
void UvUdpSocket::onSendComplete(uv_udp_send_t *req, int status)
{
    // get context
    auto *context = static_cast<SendContext *>(req->data);

    if (status < 0) {
        qDebug() << "Send failed:" << uv_strerror(status);
    }
    // delete context
    delete[] context->buffer;
    delete context;
    delete req;
}
void UvUdpSocket::sendData(const QByteArray &data, const QString &address, quint16 port)
{
    QMetaObject::invokeMethod(this,
                              "sendDataPrivate",
                              Q_ARG(const QByteArray &, data),
                              Q_ARG(const QString &, address),
                              Q_ARG(quint16, port));
}

void UvUdpSocket::closeSocket()
{
    uv_udp_recv_stop(udp_socket_.get());
    uv_close((uv_handle_t *) udp_socket_.get(), nullptr);
}

void UvUdpSocket::onUdpRead(uv_udp_t *,
                            ssize_t nread,
                            const uv_buf_t *buf,
                            const struct sockaddr *addr)
{
    if (nread > 0) {
        //qDebug() << "uvSocket recv in thread id" << gettid();
        char ip_str[INET6_ADDRSTRLEN];
        const struct sockaddr_in *ipv4_addr = (const struct sockaddr_in *) addr;
        quint16 port = ntohs(ipv4_addr->sin_port);
        uv_ip4_name(ipv4_addr, ip_str, sizeof(ip_str));
        QByteArray receivedData(buf->base, nread);

        ///////////////only for test performance///////////////////////
        g_currRecvSize += nread;
        if (receivedData[0] == 0x01) {
            qDebug() << QString("recv %1 MiB data").arg(g_currRecvSize / 1024.0 / 1024.0);
        }
        //////////////////////////////////////////////////////////////
        emit dataReceived(ip_str, port, receivedData);
    } else if (nread < 0) {
        qDebug() << "Error reading UDP data: " << uv_strerror(nread);
    }
}
void UvUdpSocket::allocBuffer(uv_handle_t *, size_t suggested_size, uv_buf_t *buf)
{
    static char slab[65536];
    buf->base = slab;
    buf->len = suggested_size;
}
void UvUdpSocket::onUdpReadStatic(
    uv_udp_s *req, long nread, const uv_buf_t *buf, const sockaddr *addr, unsigned int)
{
    UvUdpSocket *socket = static_cast<UvUdpSocket *>(uv_handle_get_data((uv_handle_t *) req));
    socket->onUdpRead((uv_udp_t *) req, (ssize_t) nread, buf, addr);
}

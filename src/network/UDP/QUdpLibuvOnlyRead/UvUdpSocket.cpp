#include "UvUdpSocket.h"
#include <QDebug>
#include <QThread>
#include <unistd.h>
using namespace shuimo;
using namespace std;
using namespace std::placeholders;
UvUdpSocket::UvUdpSocket(const QString &ip, quint16 port, QObject *parent)
    : QObject(parent)
    , ip_(ip)
    , port_(port)
{}

UvUdpSocket::~UvUdpSocket()
{
    closeSocket();
    uv_loop_close(loop_.get());
}

void UvUdpSocket::initSocket()
{
    loop_.reset(uv_loop_new());
    udp_socket_ = make_shared<uv_udp_t>();
    uv_handle_set_data((uv_handle_t *) udp_socket_.get(), this);
    uv_udp_init(loop_.get(), udp_socket_.get());
    struct sockaddr_in addr;
    uv_ip4_addr(ip_.toStdString().c_str(), port_, &addr);

    int r = uv_udp_bind(udp_socket_.get(), (const struct sockaddr *) &addr, 0);
    if (r) {
        qDebug() << "Failed to bind UDP socket: " << uv_strerror(r);
    }

    r = uv_udp_recv_start(udp_socket_.get(), alloc_buffer, on_udp_read_static);
    if (r) {
        qDebug() << "Failed to start UDP socket: " << uv_strerror(r);
    }
    uv_run(loop_.get(), UV_RUN_DEFAULT);
}

void UvUdpSocket::sendData(const QByteArray &data, const QString &address, quint16 port)
{
    struct sockaddr_in addr;
    uv_ip4_addr(address.toStdString().c_str(), port, &addr);

    uv_buf_t buf;
    buf.base = const_cast<char *>(data.constData());
    buf.len = data.length();

    int r = uv_udp_send(nullptr,
                        udp_socket_.get(),
                        &buf,
                        1,
                        (const struct sockaddr *) &addr,
                        nullptr);
    if (r) {
        qDebug() << "Failed to send UDP data: " << uv_strerror(r);
    }
}

void UvUdpSocket::closeSocket()
{
    uv_udp_recv_stop(udp_socket_.get());
    uv_close((uv_handle_t *) udp_socket_.get(), nullptr);
}

void UvUdpSocket::on_udp_read(uv_udp_t *,
                              ssize_t nread,
                              const uv_buf_t *buf,
                              const struct sockaddr *addr)
{
    if (nread > 0) {
        qDebug() << "uvSocket recv in thread id" << gettid();
        char ip_str[INET6_ADDRSTRLEN];
        const struct sockaddr_in *ipv4_addr = (const struct sockaddr_in *) addr;
        quint16 port = ntohs(ipv4_addr->sin_port);
        uv_ip4_name(ipv4_addr, ip_str, sizeof(ip_str));
        QByteArray receivedData(buf->base, nread);
        emit dataReceived(ip_str, port, receivedData);
    } else if (nread < 0) {
        qDebug() << "Error reading UDP data: " << uv_strerror(nread);
    }
    delete[] buf->base;
}
void UvUdpSocket::alloc_buffer(uv_handle_t *, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}
void UvUdpSocket::on_udp_read_static(
    uv_udp_s *req, long nread, const uv_buf_t *buf, const sockaddr *addr, unsigned int)
{
    UvUdpSocket *socket = static_cast<UvUdpSocket *>(uv_handle_get_data((uv_handle_t *) req));
    socket->on_udp_read((uv_udp_t *) req, (ssize_t) nread, buf, addr);
}

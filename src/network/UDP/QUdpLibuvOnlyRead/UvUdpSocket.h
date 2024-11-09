#ifndef UVUDPSOCKET_H
#define UVUDPSOCKET_H
///this code is demo for use libuv in QT
/// can only use libuv read in child thread
#include <QObject>
#include <memory>
#include <uv.h>
namespace shuimo {
class UvUdpSocket : public QObject
{
    Q_OBJECT
public:
    explicit UvUdpSocket(const QString &ip, quint16 port, QObject *parent = nullptr);
    ~UvUdpSocket();
    //not thread safe
    void sendData(const QByteArray &data, const QString &address, quint16 port);
public slots:
    void initSocket();
signals:
    void dataReceived(const QString &addr, quint16 port, const QByteArray &data);

private:
    void closeSocket();
    void on_udp_read(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr);
    static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    static void on_udp_read_static(
        uv_udp_s *req, long nread, const uv_buf_t *buf, const sockaddr *addr, unsigned int flags);
    Q_DISABLE_COPY(UvUdpSocket) //noncopyable UvUdpSocket
    const QString ip_;
    const quint16 port_;
    std::shared_ptr<uv_loop_t> loop_;
    std::shared_ptr<uv_udp_t> udp_socket_;
};
} // namespace shuimo

#endif // UVUDPSOCKET_H

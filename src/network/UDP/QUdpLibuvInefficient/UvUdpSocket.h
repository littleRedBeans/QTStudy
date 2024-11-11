#ifndef UVUDPSOCKET_H
#define UVUDPSOCKET_H
/// this code is demo for use libuv in QT
/// can send and recv udp data
#include <QObject>
#include <atomic>
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
private slots:
    void sendDataPrivate(const QByteArray &data, const QString &address, quint16 port);
signals:
    void dataReceived(const QString &addr, quint16 port, const QByteArray &data);

private:
    struct SendContext
    {
        char *buffer;
        uv_buf_t uv_buf;
    };
    void closeSocket();
    void onUdpRead(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr);
    static void allocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    static void onUdpReadStatic(
        uv_udp_s *req, long nread, const uv_buf_t *buf, const sockaddr *addr, unsigned int flags);
    static void onSendComplete(uv_udp_send_t *req, int status);
    Q_DISABLE_COPY(UvUdpSocket) //noncopyable UvUdpSocket
    const QString ip_;
    const quint16 port_;
    std::atomic_bool start_;
    std::shared_ptr<uv_loop_t> loop_;
    std::shared_ptr<uv_udp_t> udp_socket_;
};
} // namespace shuimo

#endif // UVUDPSOCKET_H

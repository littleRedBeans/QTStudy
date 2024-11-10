#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>
namespace shuimo {
class UdpServer : public QObject
{
    Q_OBJECT
public:
    UdpServer(const QString &ip, quint16 port, QObject *parent = nullptr);
public slots:
    void initSocket();
private slots:
    void readPendingDatagrams();
signals:
    void dataReceived(const QString &addr, quint16 port, const QByteArray &data);

private:
    QString ip_;
    quint16 port_;
    QUdpSocket socket_;
};
} // namespace shuimo

#endif // UDPSERVER_H

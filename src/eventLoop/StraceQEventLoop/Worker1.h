#ifndef WORKER1_H
#define WORKER1_H

#include <QDateTime>
#include <QObject>
class Worker1 : public QObject
{
    Q_OBJECT
public:
    explicit Worker1(QObject *parent = nullptr);
public slots:
    void logText(const QString &text);
signals:
    void sigLogText(const QString &text, const QDateTime &time);
};

#endif // WORKER1_H

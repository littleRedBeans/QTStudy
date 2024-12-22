#include "Worker1.h"
#include <sys/syscall.h>
#include <unistd.h>
pid_t gettid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}
Worker1::Worker1(QObject *parent)
    : QObject{parent}
{
}

void Worker1::logText(const QString &text)
{
    emit sigLogText(QString("threadID %1:").arg(gettid()) + text, QDateTime::currentDateTime());
}

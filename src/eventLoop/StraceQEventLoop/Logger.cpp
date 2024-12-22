#include "Logger.h"
#include <QDebug>
#include <sys/syscall.h>
#include <unistd.h>

Logger::Logger(QObject *parent)
    : QObject{parent}
{
    
}

void Logger::onLogText(QString text, QDateTime time)
{
    qDebug() << time.toString("yyyy-MM-dd HH:MM:ss zzz") << text;
}

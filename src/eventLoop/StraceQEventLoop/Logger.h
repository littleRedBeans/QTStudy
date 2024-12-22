#ifndef LOGGER_H
#define LOGGER_H

#include <QDateTime>
#include <QObject>
class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = nullptr);
public slots:
    void onLogText(QString text, QDateTime time);
signals:

};

#endif // LOGGER_H

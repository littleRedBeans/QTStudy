#include "EventDispatcherLibUv.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QSocketNotifier>
#include <QtCore/QThread>
#include <QtCore/QTimerEvent>
#include "EventDispatcherLibUvPrivate.h"

EventDispatcherLibUv::EventDispatcherLibUv(QObject *parent)
    : QAbstractEventDispatcher(parent)
    , d_ptr(new EventDispatcherLibUvPrivate(this))
{}

EventDispatcherLibUv::~EventDispatcherLibUv() = default;
uv_loop_t *EventDispatcherLibUv::uvLoop() const
{
    Q_D(const EventDispatcherLibUv);
    return d->m_base;
}
bool EventDispatcherLibUv::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(EventDispatcherLibUv);
    return d->processEvents(flags);
}

void EventDispatcherLibUv::registerSocketNotifier(QSocketNotifier *notifier)
{
    Q_D(EventDispatcherLibUv);
    d->registerSocketNotifier(notifier);
}

void EventDispatcherLibUv::unregisterSocketNotifier(QSocketNotifier *notifier)
{
    Q_D(EventDispatcherLibUv);
    d->unregisterSocketNotifier(notifier);
}

void EventDispatcherLibUv::registerTimer(int timerId,
                                         qint64 interval,
                                         Qt::TimerType timerType,
                                         QObject *object)
{
    Q_D(EventDispatcherLibUv);
    d->registerTimer(timerId, interval, timerType, object);
}

bool EventDispatcherLibUv::unregisterTimer(int timerId)
{
    Q_D(EventDispatcherLibUv);
    return d->unregisterTimer(timerId);
}

bool EventDispatcherLibUv::unregisterTimers(QObject *object)
{
    Q_D(EventDispatcherLibUv);
    return d->unregisterTimers(object);
}

QList<QAbstractEventDispatcher::TimerInfo> EventDispatcherLibUv::registeredTimers(
    QObject *object) const
{
    Q_D(const EventDispatcherLibUv);
    return d->registeredTimers(object);
}

int EventDispatcherLibUv::remainingTime(int timerId)
{
    Q_D(const EventDispatcherLibUv);
    return d->remainingTime(timerId);
}

void EventDispatcherLibUv::wakeUp()
{
    Q_D(EventDispatcherLibUv);
    if (d->m_wakeups.testAndSetAcquire(0, 1)) {
        uv_async_send(&d->m_wakeup);
    }
}

void EventDispatcherLibUv::interrupt()
{
    Q_D(EventDispatcherLibUv);
    d->m_interrupt = true;
    wakeUp();
}

void EventDispatcherLibUv::startingUp()
{
    // 启动时的初始化工作
}

void EventDispatcherLibUv::closingDown()
{
    // 关闭时的清理工作
}

EventDispatcherLibUv::EventDispatcherLibUv(EventDispatcherLibUvPrivate &dd, QObject *parent)
    : QAbstractEventDispatcher(parent)
    , d_ptr(&dd)
{}

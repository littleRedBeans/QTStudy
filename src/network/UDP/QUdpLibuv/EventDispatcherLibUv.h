#ifndef EVENTDISPATCHERLIBUV_H
#define EVENTDISPATCHERLIBUV_H

#include <QtCore/QAbstractEventDispatcher>
#include <uv.h>

class EventDispatcherLibUvPrivate;

class EventDispatcherLibUv : public QAbstractEventDispatcher
{
    Q_OBJECT
public:
    explicit EventDispatcherLibUv(QObject *parent = nullptr);
    ~EventDispatcherLibUv() override;

    bool processEvents(QEventLoop::ProcessEventsFlags flags) override;
    void registerSocketNotifier(QSocketNotifier *notifier) override;
    void unregisterSocketNotifier(QSocketNotifier *notifier) override;
    void registerTimer(int timerId,
                       qint64 interval,
                       Qt::TimerType timerType,
                       QObject *object) override;
    bool unregisterTimer(int timerId) override;
    bool unregisterTimers(QObject *object) override;
    QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject *object) const override;
    int remainingTime(int timerId) override;
    void wakeUp() override;
    void interrupt() override;
    void startingUp() override;
    void closingDown() override;

    uv_loop_t *uvLoop() const;

protected:
    EventDispatcherLibUv(EventDispatcherLibUvPrivate &dd, QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(EventDispatcherLibUv)
    Q_DECLARE_PRIVATE(EventDispatcherLibUv)
    QScopedPointer<EventDispatcherLibUvPrivate> d_ptr;
};

#endif // EVENTDISPATCHERLIBUV_H

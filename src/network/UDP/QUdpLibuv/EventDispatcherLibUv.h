#ifndef EVENTDISPATCHERLIBUV_H
#define EVENTDISPATCHERLIBUV_H
/*
Copyright (c) 2012-2013 Vladimir Kolesnikov <vladimir@free-sevastopol.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
 */
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

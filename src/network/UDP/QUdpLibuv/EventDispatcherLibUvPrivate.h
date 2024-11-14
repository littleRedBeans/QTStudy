#ifndef EVENTDISPATCHERLIBUVPRIVATE_H
#define EVENTDISPATCHERLIBUVPRIVATE_H
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
#include <QtCore/QAtomicInt>
#include <QtCore/QHash>
#include <QtCore/QPointer>
#include <uv.h>

struct TimerInfo
{
    QObject *object;
    uv_timer_t ev;
    int timerId;
    qint64 interval;
    Qt::TimerType type;
};

struct ZeroTimer
{
    QObject *object;
    bool active;
};

class EventDispatcherLibUv;

class Q_DECL_HIDDEN EventDispatcherLibUvPrivate
{
public:
    EventDispatcherLibUvPrivate(EventDispatcherLibUv *const q);
    ~EventDispatcherLibUvPrivate();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    void registerSocketNotifier(QSocketNotifier *notifier);
    void unregisterSocketNotifier(QSocketNotifier *notifier);
    void registerTimer(int timerId, qint64 interval, Qt::TimerType type, QObject *object);
    void registerZeroTimer(int timerId, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);
    QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject *object) const;
    int remainingTime(int timerId) const;

    typedef QHash<QSocketNotifier *, uv_poll_t *> SocketNotifierHash;
    typedef QHash<int, TimerInfo *> TimerHash;
    typedef QPair<QPointer<QObject>, QEvent *> PendingEvent;
    typedef QList<PendingEvent> EventList;
    typedef QHash<int, ZeroTimer> ZeroTimerHash;

private:
    Q_DISABLE_COPY(EventDispatcherLibUvPrivate)
    Q_DECLARE_PUBLIC(EventDispatcherLibUv)
    EventDispatcherLibUv *const q_ptr;

    bool m_interrupt;
    uv_loop_t *m_base;
    uv_async_t m_wakeup;
    QAtomicInt m_wakeups;
    SocketNotifierHash m_notifiers;
    TimerHash m_timers;
    EventList m_event_list;
    ZeroTimerHash m_zero_timers;
    bool m_awaken;

    static void socket_notifier_callback(uv_poll_t *w, int status, int events);
    static void timer_callback(uv_timer_t *w);
    static void wake_up_handler(uv_async_t *w);

    bool disableSocketNotifiers(bool disable);
    void killSocketNotifiers();
    bool disableTimers(bool disable);
    void killTimers();
    bool processZeroTimers();
};

#endif // EVENTDISPATCHERLIBUVPRIVATE_H

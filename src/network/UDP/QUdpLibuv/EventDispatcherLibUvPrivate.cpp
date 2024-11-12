#include "EventDispatcherLibUvPrivate.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimerEvent>
#include "EventDispatcherLibUv.h"

EventDispatcherLibUvPrivate::EventDispatcherLibUvPrivate(EventDispatcherLibUv *const q)
    : q_ptr(q)
    , m_interrupt(false)
    , m_base(new uv_loop_t)
    , m_wakeups(0)
    , m_awaken(false)
{
    uv_loop_init(m_base);
    m_base->data = this;
    uv_async_init(m_base, &m_wakeup, wake_up_handler);
}

EventDispatcherLibUvPrivate::~EventDispatcherLibUvPrivate()
{
    if (m_base) {
        killTimers();
        killSocketNotifiers();
        uv_close(reinterpret_cast<uv_handle_t *>(&m_wakeup), nullptr);
        uv_run(m_base, UV_RUN_DEFAULT);
        uv_loop_close(m_base);
        delete m_base;
        m_base = nullptr;
    }
}

bool EventDispatcherLibUvPrivate::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_Q(EventDispatcherLibUv);

    const bool exclude_notifiers = (flags & QEventLoop::ExcludeSocketNotifiers);
    const bool exclude_timers = (flags & QEventLoop::X11ExcludeTimers);

    exclude_notifiers &&disableSocketNotifiers(true);
    exclude_timers &&disableTimers(true);

    m_interrupt = false;
    m_awaken = false;

    bool result = !m_event_list.isEmpty() || !m_zero_timers.isEmpty() || !m_timers.isEmpty()
                  || !m_notifiers.isEmpty();

    Q_EMIT q->awake();
    QCoreApplication::sendPostedEvents();

    bool can_wait = !m_interrupt && (flags & QEventLoop::WaitForMoreEvents) && !result;
    uv_run_mode mode = UV_RUN_NOWAIT;

    if (!m_interrupt) {
        if (!exclude_timers && !m_zero_timers.isEmpty()) {
            result |= processZeroTimers();
            if (result) {
                can_wait = false;
            }
        }

        if (can_wait) {
            Q_EMIT q->aboutToBlock();
            mode = UV_RUN_ONCE;
        }

        uv_run(m_base, mode);

        EventList list;
        m_event_list.swap(list);
        result |= (!list.isEmpty() || m_awaken);

        for (const PendingEvent &e : list) {
            if (!e.first.isNull()) {
                QCoreApplication::sendEvent(e.first, e.second);
            }
            delete e.second;
        }
    }

    exclude_notifiers &&disableSocketNotifiers(false);
    exclude_timers &&disableTimers(false);

    return result;
}

void EventDispatcherLibUvPrivate::registerSocketNotifier(QSocketNotifier *notifier)
{
    int events = 0;
    switch (notifier->type()) {
    case QSocketNotifier::Read:
        events = UV_READABLE;
        break;
    case QSocketNotifier::Write:
        events = UV_WRITABLE;
        break;
    case QSocketNotifier::Exception:
        return;
    }

    auto poll_handle = new uv_poll_t;
    uv_poll_init_socket(m_base, poll_handle, notifier->socket());
    poll_handle->data = notifier;
    m_notifiers.insert(notifier, poll_handle);
    uv_poll_start(poll_handle, events, socket_notifier_callback);
}

void EventDispatcherLibUvPrivate::unregisterSocketNotifier(QSocketNotifier *notifier)
{
    auto it = m_notifiers.find(notifier);
    if (it != m_notifiers.end()) {
        uv_poll_t *handle = it.value();
        m_notifiers.erase(it);
        uv_close(reinterpret_cast<uv_handle_t *>(handle),
                 [](uv_handle_t *h) { delete reinterpret_cast<uv_poll_t *>(h); });
    }
}

void EventDispatcherLibUvPrivate::registerTimer(int timerId,
                                                qint64 interval,
                                                Qt::TimerType type,
                                                QObject *object)
{
    auto info = new TimerInfo{object, {}, timerId, interval, type};
    uv_timer_init(m_base, &info->ev);
    info->ev.data = info;
    m_timers.insert(timerId, info);
    uv_timer_start(&info->ev, timer_callback, interval, 0);
}

void EventDispatcherLibUvPrivate::registerZeroTimer(int timerId, QObject *object)
{
    ZeroTimer timer{object, true};
    m_zero_timers.insert(timerId, timer);
}

bool EventDispatcherLibUvPrivate::unregisterTimer(int timerId)
{
    auto it = m_timers.find(timerId);
    if (it != m_timers.end()) {
        TimerInfo *info = it.value();
        m_timers.erase(it);
        uv_timer_stop(&info->ev);
        uv_close(reinterpret_cast<uv_handle_t *>(&info->ev),
                 [](uv_handle_t *h) { delete static_cast<TimerInfo *>(h->data); });
        return true;
    }
    return m_zero_timers.remove(timerId) > 0;
}

bool EventDispatcherLibUvPrivate::unregisterTimers(QObject *object)
{
    bool result = false;
    for (auto it = m_timers.begin(); it != m_timers.end();) {
        TimerInfo *info = it.value();
        if (info->object == object) {
            uv_timer_stop(&info->ev);
            uv_close(reinterpret_cast<uv_handle_t *>(&info->ev),
                     [](uv_handle_t *h) { delete static_cast<TimerInfo *>(h->data); });
            it = m_timers.erase(it);
            result = true;
        } else {
            ++it;
        }
    }
    for (auto it = m_zero_timers.begin(); it != m_zero_timers.end();) {
        if (it.value().object == object) {
            it = m_zero_timers.erase(it);
            result = true;
        } else {
            ++it;
        }
    }
    return result;
}

QList<QAbstractEventDispatcher::TimerInfo> EventDispatcherLibUvPrivate::registeredTimers(
    QObject *object) const
{
    QList<QAbstractEventDispatcher::TimerInfo> result;
    for (const auto &info : m_timers) {
        if (info->object == object) {
            result.append({info->timerId, static_cast<int>(info->interval), info->type});
        }
    }
    for (auto it = m_zero_timers.begin(); it != m_zero_timers.end(); ++it) {
        if (it.value().object == object) {
            result.append({it.key(), 0, Qt::PreciseTimer});
        }
    }
    return result;
}

int EventDispatcherLibUvPrivate::remainingTime(int timerId) const
{
    auto it = m_timers.find(timerId);
    if (it != m_timers.end()) {
        TimerInfo *info = it.value();
        return static_cast<int>(info->interval);
    }
    return -1;
}

void EventDispatcherLibUvPrivate::socket_notifier_callback(uv_poll_t *w, int status, int events)
{
    QSocketNotifier *notifier = static_cast<QSocketNotifier *>(w->data);
    EventDispatcherLibUvPrivate *self = static_cast<EventDispatcherLibUvPrivate *>(w->loop->data);

    if (status < 0) {
        qWarning("Socket notifier error: %s", uv_strerror(status));
        return;
    }

    QSocketNotifier::Type type = notifier->type();
    if ((type == QSocketNotifier::Read && (events & UV_READABLE))
        || (type == QSocketNotifier::Write && (events & UV_WRITABLE))) {
        // 使用 QSocketNotifier 的 event() 方法来创建事件
        QEvent *event = new QEvent(QEvent::SockAct);
        self->m_event_list.append(PendingEvent(notifier, event));
    }
}

void EventDispatcherLibUvPrivate::timer_callback(uv_timer_t *w)
{
    TimerInfo *info = static_cast<TimerInfo *>(w->data);
    EventDispatcherLibUvPrivate *self = static_cast<EventDispatcherLibUvPrivate *>(w->loop->data);
    self->m_event_list.append(PendingEvent(info->object, new QTimerEvent(info->timerId)));
}

void EventDispatcherLibUvPrivate::wake_up_handler(uv_async_t *w)
{
    EventDispatcherLibUvPrivate *self = static_cast<EventDispatcherLibUvPrivate *>(w->loop->data);
    self->m_awaken = true;
    self->m_wakeups.storeRelease(0);
}

bool EventDispatcherLibUvPrivate::processZeroTimers()
{
    bool result = false;
    QList<int> ids = m_zero_timers.keys();

    for (int timerId : ids) {
        auto it = m_zero_timers.find(timerId);
        if (it != m_zero_timers.end()) {
            ZeroTimer &data = it.value();
            if (data.active) {
                data.active = false;
                QTimerEvent event(timerId);
                QCoreApplication::sendEvent(data.object, &event);
                result = true;

                it = m_zero_timers.find(timerId);
                if (it != m_zero_timers.end()) {
                    it.value().active = true;
                }
            }
        }
    }

    return result;
}

bool EventDispatcherLibUvPrivate::disableSocketNotifiers(bool disable)
{
    for (auto it = m_notifiers.begin(); it != m_notifiers.end(); ++it) {
        uv_poll_t *handle = it.value();
        if (disable) {
            uv_poll_stop(handle);
        } else {
            QSocketNotifier *notifier = it.key();
            int events = 0;
            switch (notifier->type()) {
            case QSocketNotifier::Read:
                events = UV_READABLE;
                break;
            case QSocketNotifier::Write:
                events = UV_WRITABLE;
                break;
            default:
                continue;
            }
            uv_poll_start(handle, events, socket_notifier_callback);
        }
    }
    return true;
}

bool EventDispatcherLibUvPrivate::disableTimers(bool disable)
{
    for (auto it = m_timers.begin(); it != m_timers.end(); ++it) {
        TimerInfo *info = it.value();
        if (disable) {
            uv_timer_stop(&info->ev);
        } else {
            uv_timer_start(&info->ev, timer_callback, info->interval, 0);
        }
    }
    return true;
}

void EventDispatcherLibUvPrivate::killSocketNotifiers()
{
    for (auto it = m_notifiers.begin(); it != m_notifiers.end(); ++it) {
        uv_poll_t *handle = it.value();
        uv_close(reinterpret_cast<uv_handle_t *>(handle),
                 [](uv_handle_t *h) { delete reinterpret_cast<uv_poll_t *>(h); });
    }
    m_notifiers.clear();
}

void EventDispatcherLibUvPrivate::killTimers()
{
    for (auto it = m_timers.begin(); it != m_timers.end(); ++it) {
        TimerInfo *info = it.value();
        uv_timer_stop(&info->ev);
        uv_close(reinterpret_cast<uv_handle_t *>(&info->ev),
                 [](uv_handle_t *h) { delete static_cast<TimerInfo *>(h->data); });
    }
    m_timers.clear();
    m_zero_timers.clear();
}

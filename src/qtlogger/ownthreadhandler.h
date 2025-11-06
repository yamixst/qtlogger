#pragma once

#include <type_traits>

#include <QCoreApplication>
#include <QEvent>
#include <QObject>
#include <QPointer>
#include <QThread>

#include "handler.h"
#include "logger_global.h"
#include "logmessage.h"

namespace QtLogger {

template<typename BaseHandler>
class QTLOGGER_EXPORT OwnThreadHandler : public BaseHandler
{
    static_assert(std::is_base_of<Handler, BaseHandler>::value,
                  "BaseHandler must inherit from Handler");

public:
    template<typename... Args>
    OwnThreadHandler(Args &&...args) : BaseHandler(std::forward<Args>(args)...)
    {
        static auto __once = qRegisterMetaType<QtLogger::LogMessage>("QtLogger::LogMessage");
        Q_UNUSED(__once)
    }

    ~OwnThreadHandler() override { resetOwnThread(); }

    QThread *ownThread() const { return m_thread; }

    bool ownThreadIsRunning() const { return m_thread && m_thread->isRunning(); }

    OwnThreadHandler<BaseHandler> &moveToOwnThread()
    {
        if (m_thread)
            return *this;

        m_thread = new QThread();

        if (qApp) {
            // The thread object must be guaranteed to be attached to the main thread regardless of
            // where this method was called from
            if (qApp->thread() != m_thread->thread()) {
                m_thread->moveToThread(qApp->thread());
            }
            QObject::connect(qApp, &QCoreApplication::aboutToQuit, m_thread, &QThread::quit);
        }

        QObject::connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);

        m_worker = new Worker(/* handler (not parent!) */ this);
        m_worker->moveToThread(m_thread);

        // The deletion of the worker occurs only in this place after the thread stops, so we can
        // keep a pointer to it via closures
        const auto worker = m_worker;
        QObject::connect(m_thread, &QThread::finished, [worker]() {
            // We cannot delete worker via deleteLater because m_thread will not process incoming
            // events anymore
            delete worker;
        });

        m_thread->start();

        return *this;
    }

    void resetOwnThread()
    {
        if (!m_thread)
            return;

        m_thread->quit();
        m_thread.clear();

        m_worker = nullptr;
    }

    bool process(LogMessage &lmsg) override
    {
        if (m_worker) {
            QCoreApplication::postEvent(m_worker, new LogEvent(lmsg));
        } else {
            BaseHandler::process(lmsg);
        }
        return true;
    }

private:
    struct LogEvent : public QEvent
    {
        LogEvent(const LogMessage &lmsg) : QEvent(type()), lmsg(lmsg) { }

        static QEvent::Type type()
        {
            static QEvent::Type _type = static_cast<QEvent::Type>(QEvent::registerEventType());
            return _type;
        }

        LogMessage lmsg;
    };

    class Worker : public QObject
    {
    public:
        explicit Worker(OwnThreadHandler<BaseHandler> *handler) : QObject(), m_handler(handler) { }

        void customEvent(QEvent *event) override
        {
            if (event->type() == LogEvent::type()) {
                auto logEvent = dynamic_cast<LogEvent *>(event);
                if (logEvent) {
                    m_handler->BaseHandler::process(logEvent->lmsg);
                }
            }
        }

    private:
        OwnThreadHandler<BaseHandler> *m_handler;
    };

private:
    QPointer<QThread> m_thread;
    Worker *m_worker = nullptr;
};

} // namespace QtLogger

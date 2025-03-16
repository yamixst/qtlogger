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

    ~OwnThreadHandler() override { reset(); }

    OwnThreadHandler<BaseHandler> &moveToOwnThread()
    {
        reset();

        if (!m_worker) {
            m_worker = new Worker(this);
        }

        if (!m_thread) {
            m_thread = new QThread();

            if (qApp) {
                if (qApp->thread() != m_thread->thread()) {
                    m_thread->moveToThread(qApp->thread());
                }
                QObject::connect(qApp, &QCoreApplication::aboutToQuit, m_thread, &QThread::quit);
            }

            QObject::connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
            QObject::connect(m_thread, &QThread::finished, m_worker, &QThread::deleteLater);

            m_thread->start();
        }

        m_worker->moveToThread(m_thread);

        return *this;
    }

    OwnThreadHandler<BaseHandler> &moveToMainThread()
    {
        reset();

        if (!m_worker) {
            m_worker = new Worker(this);
        }

        m_worker->moveToThread(qApp->thread());

        return *this;
    }

    bool ownThreadIsRunning() const { return m_thread && m_thread->isRunning(); }

    QThread *ownThread() const { return m_thread; }

    void reset()
    {
        if (m_thread) {
            m_thread->quit();
            m_thread->wait(100);
            if (m_thread && m_thread->isRunning()) {
                m_thread->terminate();
            }
            if (m_thread) {
                delete m_thread;
            }
        }

        if (m_worker) {
            delete m_worker;
        }
    }

    bool process(LogMessage &lmsg) override
    {
        if (m_worker) {
            QCoreApplication::postEvent(m_worker, new LogMsgEvent(lmsg));
        } else {
            BaseHandler::process(lmsg);
        }
        return true;
    }

private:
    static QEvent::Type getLogMsgEventType()
    {
        static QEvent::Type type = static_cast<QEvent::Type>(QEvent::registerEventType());
        return type;
    }

    struct LogMsgEvent : public QEvent
    {
        LogMessage lmsg;
        LogMsgEvent(const LogMessage &lmsg) : QEvent(getLogMsgEventType()), lmsg(lmsg) { }
    };

    class Worker : public QObject
    {
    public:
        explicit Worker(OwnThreadHandler<BaseHandler> *handler) : QObject(), m_handler(handler) { }

        void customEvent(QEvent *event) override
        {
            if (event->type() == getLogMsgEventType()) {
                auto lmEvent = dynamic_cast<LogMsgEvent *>(event);
                if (lmEvent) {
                    m_handler->BaseHandler::process(lmEvent->lmsg);
                }
            }
        }

    private:
        OwnThreadHandler<BaseHandler> *m_handler;
    };

    QPointer<Worker> m_worker;
    QPointer<QThread> m_thread;
};

} // namespace QtLogger

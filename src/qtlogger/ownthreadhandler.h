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

    OwnThreadHandler &moveToOwnThread()
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

    OwnThreadHandler &moveToMainThread()
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

    bool process(LogMessage &lmsg) override
    {
        if (m_worker) {
            QCoreApplication::postEvent(m_worker, new ProcLogMsgEvent(lmsg));
        } else {
            BaseHandler::process(lmsg);
        }
        return true;
    }

private:
    class Worker;

    QPointer<Worker> m_worker;
    QPointer<QThread> m_thread;

    struct ProcLogMsgEvent : public QEvent
    {
        LogMessage lmsg;
        ProcLogMsgEvent(const LogMessage &lmsg) : QEvent(getProcLogMsgEventType()), lmsg(lmsg) { }
    };

    static QEvent::Type getProcLogMsgEventType()
    {
        static QEvent::Type type = static_cast<QEvent::Type>(QEvent::registerEventType());
        return type;
    }
};

template<typename BaseHandler>
class OwnThreadHandler<BaseHandler>::Worker : public QObject
{
public:
    explicit Worker(OwnThreadHandler<BaseHandler> *handler) : QObject(), m_handler(handler) { }

    void customEvent(QEvent *event) override
    {
        if (event->type() == OwnThreadHandler<BaseHandler>::getProcLogMsgEventType()) {
            auto ev =
                    dynamic_cast<typename OwnThreadHandler<BaseHandler>::ProcLogMsgEvent *>(event);
            if (ev) {
                m_handler->BaseHandler::process(ev->lmsg);
            }
        }
    }

private:
    OwnThreadHandler<BaseHandler> *m_handler;
};

} // namespace QtLogger

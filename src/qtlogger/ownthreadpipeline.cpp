#include "ownthreadpipeline.h"

#include <QCoreApplication>
#include <QEvent>
#include <QThread>

namespace QtLogger {

namespace {

static QEvent::Type getProcessLogMessageEventType()
{
    static QEvent::Type type = static_cast<QEvent::Type>(QEvent::registerEventType());
    return type;
}

struct QTLOGGER_EXPORT ProcessLogMessageEvent : public QEvent
{
    LogMessage lmsg;
    ProcessLogMessageEvent(const LogMessage &lmsg)
        : QEvent(getProcessLogMessageEventType()), lmsg(lmsg)
    {
    }
};

}

class QTLOGGER_EXPORT OwnThreadPipeline::Worker : public QObject
{
public:
    explicit Worker(OwnThreadPipeline *handler, QObject *parent = nullptr)
        : QObject(parent), m_handler(handler)
    {
    }

    void customEvent(QEvent *event) override
    {
        if (event->type() == getProcessLogMessageEventType()) {
            auto ev = dynamic_cast<ProcessLogMessageEvent *>(event);
            if (ev) {
                m_handler->SimplePipeline::process(ev->lmsg);
            }
        }
    }

private:
    OwnThreadPipeline *m_handler;
};

QTLOGGER_DECL_SPEC
OwnThreadPipeline::OwnThreadPipeline()
{
    static auto __once = qRegisterMetaType<QtLogger::LogMessage>("QtLogger::LogMessage");
    Q_UNUSED(__once)
}

QTLOGGER_DECL_SPEC
OwnThreadPipeline::~OwnThreadPipeline()
{
#ifndef QTLOGGER_NO_THREAD
    if (m_thread) {
        m_thread->quit();
        m_thread->wait(1000);
        if (m_thread)
            delete m_thread;
    }

    if (m_worker) {
        delete m_worker;
    }
#endif
}

QTLOGGER_DECL_SPEC
OwnThreadPipeline &OwnThreadPipeline::moveToOwnThread()
{
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

QTLOGGER_DECL_SPEC
void OwnThreadPipeline::moveToMainThread()
{
    m_worker->moveToThread(qApp->thread());

    if (m_thread) {
        m_thread->quit();
    }
}

QTLOGGER_DECL_SPEC
bool OwnThreadPipeline::ownThreadIsRunning() const
{
    return m_thread && m_thread->isRunning();
}

QTLOGGER_DECL_SPEC
bool OwnThreadPipeline::process(LogMessage &lmsg)
{
    if (!ownThreadIsRunning()) {
        SimplePipeline::process(lmsg);
    } else {
        QCoreApplication::postEvent(m_worker, new ProcessLogMessageEvent(lmsg));
    }
    return true;
}

} // namespace QtLogger

#include "ownthreadpipeline.h"

#include <QCoreApplication>

namespace QtLogger {

namespace OwnThreadPipelinePrivate {

QTLOGGER_DECL_SPEC
static QEvent::Type processLogMessageEventType()
{
    static QEvent::Type type = static_cast<QEvent::Type>(QEvent::registerEventType());
    return type;
}

struct LogEvent : public QEvent
{
    LogMessage logMsg;
    LogEvent(const LogMessage &logMsg) : QEvent(processLogMessageEventType()), logMsg(logMsg) { }
};
}

QTLOGGER_DECL_SPEC
OwnThreadPipeline::OwnThreadPipeline()
{
    qRegisterMetaType<QtLogger::LogMessage>("QtLogger::LogMessage");
}

QTLOGGER_DECL_SPEC
OwnThreadPipeline::~OwnThreadPipeline()
{
#ifndef QTLOGGER_NO_THREAD
    if (m_thread) {
        m_thread->quit();
    }
#endif
}

QTLOGGER_DECL_SPEC
void OwnThreadPipeline::moveToOwnThread()
{
    if (!m_thread) {
        m_thread = new QThread;
        if (qApp) {
            if (qApp->thread() != m_thread->thread())
                m_thread->moveToThread(qApp->thread());
            connect(qApp, &QCoreApplication::aboutToQuit, m_thread, &QThread::quit);
        }
        connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
        m_thread->start();
    }

    moveToThread(m_thread);
}

QTLOGGER_DECL_SPEC
void OwnThreadPipeline::moveToMainThread()
{
    moveToThread(qApp->thread());

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
void OwnThreadPipeline::customEvent(QEvent *event)
{
    if (event->type() == OwnThreadPipelinePrivate::processLogMessageEventType()) {
        auto ev = dynamic_cast<OwnThreadPipelinePrivate::LogEvent *>(event);
        if (ev) {
            TypedPipeline::process(ev->logMsg);
        }
    }
}

QTLOGGER_DECL_SPEC
bool OwnThreadPipeline::process(LogMessage &logMsg)
{
    if (!ownThreadIsRunning()) {
        TypedPipeline::process(logMsg);
    } else {
        QCoreApplication::postEvent(this, new OwnThreadPipelinePrivate::LogEvent(logMsg));
    }
    return true;
}

} // namespace QtLogger

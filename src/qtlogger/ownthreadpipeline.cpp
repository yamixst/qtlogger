#include "ownthreadpipeline.h"

#include <QCoreApplication>
#include <QEvent>
#include <QThread>

namespace QtLogger {



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
    if (!m_worker) {
        m_worker = new OwnThreadPipelineWorker(this);
    }

    if (!m_thread) {
        m_thread = new QThread;

        if (qApp) {
            if (qApp->thread() != m_thread->thread())
                m_thread->moveToThread(qApp->thread());
            QObject::connect(qApp, &QCoreApplication::aboutToQuit, m_thread, &QThread::quit);
        }

        QObject::connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);

        m_thread->start();
    }

    m_worker->moveToThread(m_thread);
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
bool OwnThreadPipeline::process(LogMessage &logMsg)
{
    if (!ownThreadIsRunning()) {
        TypedPipeline::process(logMsg);
    } else {
        QCoreApplication::postEvent(m_worker, new ProcessLogMessageEvent(logMsg));
    }
    return true;
}

} // namespace QtLogger

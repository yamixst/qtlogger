#pragma once

#include <QPointer>

#include "logger_global.h"
#include "simplepipeline.h"

QT_FORWARD_DECLARE_CLASS(QThread)

namespace QtLogger {

class OwnThreadPipelineWorker;

class QTLOGGER_EXPORT OwnThreadPipeline : public SimplePipeline
{
public:
    OwnThreadPipeline();
    ~OwnThreadPipeline() override;

    OwnThreadPipeline &moveToOwnThread();
    void moveToMainThread();
    bool ownThreadIsRunning() const;
    QThread *ownThread() const { return m_thread; }

    bool process(LogMessage &lmsg) override;

private:
    QPointer<OwnThreadPipelineWorker> m_worker;
    QPointer<QThread> m_thread;
};

} // namespace QtLogger

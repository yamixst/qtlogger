#pragma once

#include <QPointer>

#include "logger_global.h"
#include "typedpipeline.h"

QT_FORWARD_DECLARE_CLASS(QThread)

namespace QtLogger {

class OwnThreadPipelineWorker;

class QTLOGGER_EXPORT OwnThreadPipeline : public TypedPipeline
{
public:
    OwnThreadPipeline();
    ~OwnThreadPipeline() override;

    void moveToOwnThread();
    void moveToMainThread();
    bool ownThreadIsRunning() const;
    QThread *ownThread() const { return m_thread; }

    bool process(LogMessage &logMsg) override;

private:
    QPointer<OwnThreadPipelineWorker> m_worker;
    QPointer<QThread> m_thread;
};

} // namespace QtLogger

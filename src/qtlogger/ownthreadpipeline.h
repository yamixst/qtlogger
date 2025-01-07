#pragma once

#include <QPointer>
#include <QThread>

#include "logger_global.h"
#include "typedpipeline.h"

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

    bool process(LogMessage &logMsg) override;

private:
    QPointer<OwnThreadPipelineWorker> m_worker;
    QPointer<QThread> m_thread;
};

} // namespace QtLogger

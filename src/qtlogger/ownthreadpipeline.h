#pragma once

#include <QEvent>
#include <QPointer>
#include <QThread>

#include "logger_global.h"
#include "typedpipeline.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
static QEvent::Type processLogMessageEventType()
{
    static QEvent::Type type = static_cast<QEvent::Type>(QEvent::registerEventType());
    return type;
}

struct QTLOGGER_EXPORT ProcessLogMessageEvent : public QEvent
{
    LogMessage logMsg;
    ProcessLogMessageEvent(const LogMessage &logMsg)
        : QEvent(processLogMessageEventType()), logMsg(logMsg)
    {
    }
};

class QTLOGGER_EXPORT OwnThreadPipelineWorker : public QObject
{
    Q_OBJECT

public:
    explicit OwnThreadPipelineWorker(Handler *handler, QObject *parent = nullptr)
        : QObject(parent), m_handler(handler)
    {
    }

    void customEvent(QEvent *event) override
    {
        if (event->type() == processLogMessageEventType()) {
            auto ev = dynamic_cast<ProcessLogMessageEvent *>(event);
            if (ev) {
                dynamic_cast<TypedPipeline *>(m_handler)->process(ev->logMsg);
            }
        }
    }

private:
    Handler *m_handler;
};

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

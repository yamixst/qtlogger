#pragma once

#include <QPointer>
#include <QThread>
#include <QEvent>
#include <QObject>

#include "typedpipeline.h"

namespace QtLogger {

class OwnThreadPipeline : public QObject, public TypedPipeline
{
    Q_OBJECT

public:
    OwnThreadPipeline();
    ~OwnThreadPipeline() override;

    void moveToOwnThread();
    void moveToMainThread();
    bool ownThreadIsRunning() const;
    inline QThread *ownThread() { return m_thread.data(); }

    bool process(LogMessage &logMsg) override;

protected:
    void customEvent(QEvent *event) override;

private:
    QPointer<QThread> m_thread;
};

} // namespace QtLogger

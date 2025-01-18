#pragma once

#ifdef Q_OS_WIN

#include <QSharedPointer>

#include "../logger_global.h"
#include "../sink.h"

namespace QtLogger {

class QTLOGGER_EXPORT WinDebugSink : public Sink
{
public:
    void send(const LogMessage &lmsg) override;
};

using WinDebugSinkPtr = QSharedPointer<WinDebugSink>;

} // namespace QtLogger

#endif

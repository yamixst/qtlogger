// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#ifdef QTLOGGER_SYSLOG

#include <QSharedPointer>

#include "../abstractmessagesink.h"
#include "../logger_global.h"

#define QTLOGGER_SYSLOG_LOG_PID  0x01
#define QTLOGGER_SYSLOG_LOG_USER (1 << 3)

namespace QtLogger {

class QTLOGGER_EXPORT SysLogSink : public AbstractMessageSink
{
public:
    explicit SysLogSink(const QString &ident, int option = QTLOGGER_SYSLOG_LOG_PID,
                        int facility = QTLOGGER_SYSLOG_LOG_USER);
    ~SysLogSink();

    void send(const LogMessage &logMsg) override;
};

using SysLogSinkPtr = QSharedPointer<SysLogSink>;

} // namespace QtLogger

#endif // QTLOGGER_SYSLOG

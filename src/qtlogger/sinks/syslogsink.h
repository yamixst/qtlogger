// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#ifdef QTLOGGER_SYSLOG

#include <QSharedPointer>

#include "../logger_global.h"
#include "../sink.h"

// Syslog options
// See syslog.h for more information
#define QTLOGGER_SYSLOG_LOG_PID  0x01
#define QTLOGGER_SYSLOG_LOG_USER (1 << 3)

namespace QtLogger {

class QTLOGGER_EXPORT SyslogSink : public Sink
{
public:
    explicit SyslogSink(const QString &ident, int option = QTLOGGER_SYSLOG_LOG_PID,
                        int facility = QTLOGGER_SYSLOG_LOG_USER);
    ~SyslogSink();

    void send(const LogMessage &logMsg) override;
};

using SyslogSinkPtr = QSharedPointer<SyslogSink>;

} // namespace QtLogger

#endif // QTLOGGER_SYSLOG

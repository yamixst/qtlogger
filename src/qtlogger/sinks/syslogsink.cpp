// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#ifdef QTLOGGER_SYSLOG

#include "syslogsink.h"

#include <syslog.h>

namespace QtLogger {

QTLOGGER_DECL_SPEC
SysLogSink::SysLogSink(const QString &ident, int option, int facility)
{
    openlog(qPrintable(ident), option, facility);
}

QTLOGGER_DECL_SPEC
SysLogSink::~SysLogSink()
{
    closelog();
}

QTLOGGER_DECL_SPEC
void SysLogSink::send(const LogMessage &logMsg)
{
    QString formattedMessage;

    if (qstrcmp(logMsg.category(), "default") == 0) {
        formattedMessage = logMsg.message();
    } else {
        formattedMessage = QStringLiteral("%1: %2").arg(logMsg.category(), logMsg.message());
    }

    int priority = LOG_DEBUG;

    switch (logMsg.type()) {
    case QtDebugMsg:
        priority = LOG_DEBUG;
        break;
    case QtWarningMsg:
        priority = LOG_WARNING;
        break;
    case QtCriticalMsg:
        priority = LOG_ERR;
        break;
    case QtFatalMsg:
        priority = LOG_EMERG;
        break;
    case QtInfoMsg:
        priority = LOG_INFO;
        break;
    default:
        return;
    }

    syslog(priority, "%s", qPrintable(formattedMessage));
}

} // namespace QtLogger

#endif // QTLOGGER_SYSLOG

// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#ifdef QTLOGGER_SYSLOG

#include "syslogsink.h"

#include <syslog.h>

namespace QtLogger {

QTLOGGER_DECL_SPEC
SyslogSink::SyslogSink(const QString &ident, int option, int facility)
{
    openlog(qPrintable(ident), option, facility);
}

QTLOGGER_DECL_SPEC
SyslogSink::~SyslogSink()
{
    closelog();
}

QTLOGGER_DECL_SPEC
void SyslogSink::send(const LogMessage &lmsg)
{
    auto priority = LOG_DEBUG;

    switch (lmsg.type()) {
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

    QString formattedMessage;
    if (qstrcmp(lmsg.category(), "default") == 0) {
        formattedMessage = lmsg.message();
    } else {
        formattedMessage = QStringLiteral("%1: %2").arg(lmsg.category(), lmsg.message());
    }

    syslog(priority, "%s", qPrintable(formattedMessage));
}

} // namespace QtLogger

#endif // QTLOGGER_SYSLOG

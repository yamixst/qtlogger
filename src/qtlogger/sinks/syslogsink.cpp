// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#ifdef QTLOGGER_SYSLOG

#include "syslogsink.h"

#ifdef QTLOGGER_SYSLOG
#    include <syslog.h>
#endif

namespace QtLogger {

QTLOGGER_DECL_SPEC
SysLogSink::SysLogSink(const QString &ident, int option, int facility)
{
#ifdef QTLOGGER_SYSLOG
    openlog(qPrintable(ident), option, facility);
#else
    Q_UNUSED(ident);
    Q_UNUSED(option);
    Q_UNUSED(facility);
#endif
}

QTLOGGER_DECL_SPEC
SysLogSink::~SysLogSink()
{
#ifdef QTLOGGER_SYSLOG
    closelog();
#endif
}

QTLOGGER_DECL_SPEC
void SysLogSink::send(const DebugMessage &dmesg)
{
#ifdef QTLOGGER_SYSLOG
    QString formattedMessage;
    if (qstrcmp(dmesg.category(), "default") == 0)
        formattedMessage = dmesg.message();
    else
        formattedMessage = QStringLiteral("%1: %2").arg(dmesg.category(), dmesg.message());

    int priority = LOG_DEBUG;

    switch (dmesg.type()) {
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
#else
    Q_UNUSED(dmesg);
#endif
}

} // namespace QtLogger

#endif // QTLOGGER_SYSLOG

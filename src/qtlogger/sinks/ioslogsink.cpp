// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#ifdef QTLOGGER_IOSLOG

#include "ioslogsink.h"

#include <os/log.h>

namespace QtLogger {

QTLOGGER_DECL_SPEC
void IosLogSink::send(const LogMessage &logMsg)
{
    QString formattedMessage;

    if (qstrcmp(logMsg.category(), "default") == 0) {
        formattedMessage = logMsg.message();
    } else {
        formattedMessage = QStringLiteral("%1: %2").arg(logMsg.category(), logMsg.message());
    }

    os_log_type_t type = OS_LOG_TYPE_DEBUG;
    switch (logMsg.type()) {
    case QtDebugMsg:
        type = OS_LOG_TYPE_DEBUG;
        break;
    case QtInfoMsg:
        type = OS_LOG_TYPE_INFO;
        break;
    case QtWarningMsg:
        type = OS_LOG_TYPE_ERROR;
        break;
    case QtCriticalMsg:
        type = OS_LOG_TYPE_FAULT;
        break;
    case QtFatalMsg:
        type = OS_LOG_TYPE_FAULT;
        break;
    };

    os_log_with_type(OS_LOG_DEFAULT, type, "%s\n", qPrintable(logMsg.message()));
}

} // namespace QtLogger

#endif // QTLOGGER_IOSLOG

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#ifdef QTLOGGER_OSLOG

#include "oslogsink.h"

#include <os/log.h>

#include <QCoreApplication>

namespace QtLogger {

QTLOGGER_DECL_SPEC
void OslogSink::send(const LogMessage &logMsg)
{
    auto customLog = os_log_create(qPrintable(QCoreApplication::applicationName()),
                                   qPrintable(logMsg.category()));

    auto type = OS_LOG_TYPE_DEBUG;
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

    os_log_with_type(customLog, type, "%s", qPrintable(logMsg.message()));
}

} // namespace QtLogger

#endif // QTLOGGER_OSLOG

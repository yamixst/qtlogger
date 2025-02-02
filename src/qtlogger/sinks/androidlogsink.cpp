// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#ifdef QTLOGGER_ANDROIDLOG

#include "androidlogsink.h"

#include <android/log.h>

namespace QtLogger {

QTLOGGER_DECL_SPEC
void AndroidLogSink::send(const LogMessage &logMsg)
{
    auto priority = ANDROID_LOG_DEBUG;

    switch (logMsg.type()) {
    case QtDebugMsg:
        priority = ANDROID_LOG_DEBUG;
        break;
    case QtInfoMsg:
        priority = ANDROID_LOG_INFO;
        break;
    case QtWarningMsg:
        priority = ANDROID_LOG_WARN;
        break;
    case QtCriticalMsg:
        priority = ANDROID_LOG_ERROR;
        break;
    case QtFatalMsg:
        priority = ANDROID_LOG_FATAL;
        break;
    };

    __android_log_print(priority, logMsg.category(), "%s", qPrintable(logMsg.message()));

    // TODO: use __android_log_write_log_message for API level 30 and above
}

} // namespace QtLogger

#endif // QTLOGGER_ANDROIDLOG

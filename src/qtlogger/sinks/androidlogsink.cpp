// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#ifdef QTLOGGER_ANDROIDLOG

#include "androidlogsink.h"

#ifdef QTLOGGER_ANDROIDLOG
#    include <android/log.h>
#endif

namespace QtLogger {

QTLOGGER_DECL_SPEC
AndroidLogSink::AndroidLogSink() {}

QTLOGGER_DECL_SPEC
void AndroidLogSink::send(const DebugMessage &dmesg)
{
#ifdef QTLOGGER_ANDROIDLOG
    android_LogPriority priority = ANDROID_LOG_DEBUG;
    switch (dmesg.type()) {
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

    __android_log_print(priority, dmesg.category(), "%s\n", qPrintable(dmesg.message()));
#else
    Q_UNUSED(dmesg);
#endif
}

} // namespace QtLogger

#endif // QTLOGGER_ANDROIDLOG

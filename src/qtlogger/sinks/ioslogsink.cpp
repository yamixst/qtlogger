// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#ifdef QTLOGGER_IOSLOG

#include "ioslogsink.h"

#ifdef QTLOGGER_IOSLOG
#    include <os/log.h>
#endif

namespace QtLogger {

QTLOGGER_DECL_SPEC
IosLogSink::IosLogSink() { }

QTLOGGER_DECL_SPEC
void IosLogSink::send(const DebugMessage &dmesg)
{
#ifdef QTLOGGER_IOSLOG
    QString formattedMessage;
    if (qstrcmp(dmesg.category(), "default") == 0)
        formattedMessage = dmesg.message();
    else
        formattedMessage = QStringLiteral("%1: %2").arg(dmesg.category(), dmesg.message());

    os_log_type_t type = OS_LOG_TYPE_DEBUG;
    switch (dmesg.type()) {
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

    os_log_with_type(OS_LOG_DEFAULT, type, "%s\n", qPrintable(dmesg.message()));
#else
    Q_UNUSED(dmesg);
#endif
}

} // namespace QtLogger

#endif // QTLOGGER_IOSLOG

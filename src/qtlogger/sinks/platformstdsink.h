// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#if defined(QTLOGGER_ANDROIDLOG)
#    include "androidlogsink.h"
#elif defined(QTLOGGER_IOSLOG)
#    include "ioslogsink.h"
#else
#    include "stderrsink.h"
#endif

namespace QtLogger {

#if defined(QTLOGGER_ANDROIDLOG)
using PlatformStdSink = AndroidLogSink;
#elif defined(QTLOGGER_IOSLOG)
using PlatformStdSink = IosLogSink;
#else
using PlatformStdSink = StdErrSink;
#endif

using PlatformStdSinkPtr = QSharedPointer<PlatformStdSink>;

}

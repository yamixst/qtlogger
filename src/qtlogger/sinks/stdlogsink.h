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
using StdLogSink = AndroidLogSink;
#elif defined(QTLOGGER_IOSLOG)
using StdLogSink = IosLogSink;
#else
using StdLogSink = StdErrSink;
#endif

using StdLogSinkPtr = QSharedPointer<StdLogSink>;

}

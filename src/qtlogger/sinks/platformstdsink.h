// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QSharedPointer>

#if defined(QTLOGGER_ANDROIDLOG)
#    include "androidlogsink.h"
#elif defined(QTLOGGER_OSLOG)
#    include "oslogsink.h"
#else
#    include "stderrsink.h"
#endif

namespace QtLogger {

#if defined(QTLOGGER_ANDROIDLOG)
using PlatformStdSink = AndroidLogSink;
#elif defined(QTLOGGER_OSLOG)
using PlatformStdSink = OslogSink;
#else
using PlatformStdSink = StdErrSink;
#endif

using PlatformStdSinkPtr = QSharedPointer<PlatformStdSink>;

}

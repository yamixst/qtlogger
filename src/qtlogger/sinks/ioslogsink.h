// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#ifdef QTLOGGER_IOSLOG

#include <QSharedPointer>

#include "../abstractmessagesink.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT IosLogSink : public Sink
{
public:
    IosLogSink();

    void send(const LogMessage &logMsg);
};

using IosLogSinkPtr = QSharedPointer<IosLogSink>;

} // namespace QtLogger

#endif // QTLOGGER_IOSLOG

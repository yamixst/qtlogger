// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#ifdef QTLOGGER_ANDROIDLOG

#include <QSharedPointer>

#include "../sink.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT AndroidLogSink : public Sink
{
public:
    void send(const LogMessage &logMsg) override;
};

using AndroidLogSinkPtr = QSharedPointer<AndroidLogSink>;

} // namespace QtLogger

#endif // QTLOGGER_ANDROIDLOG

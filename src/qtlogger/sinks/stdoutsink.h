// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "../sink.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT StdOutSink : public Sink
{
public:
    StdOutSink();

    void send(const LogMessage &logMsg) override;
};

using StdOutSinkPtr = QSharedPointer<StdOutSink>;

} // namespace QtLogger

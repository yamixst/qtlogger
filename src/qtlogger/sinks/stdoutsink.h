// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "../logger_global.h"
#include "../sink.h"

namespace QtLogger {

class QTLOGGER_EXPORT StdOutSink : public Sink
{
public:
    void send(const LogMessage &logMsg) override;
    virtual bool flush() override;
};

using StdOutSinkPtr = QSharedPointer<StdOutSink>;

} // namespace QtLogger

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "../logger_global.h"
#include "../sink.h"

namespace QtLogger {

class QTLOGGER_EXPORT StdErrSink : public Sink
{
public:
    void send(const LogMessage &logMsg) override;
};

using StdErrSinkPtr = QSharedPointer<StdErrSink>;

} // namespace QtLogger

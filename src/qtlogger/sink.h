// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "handler.h"
#include "logger_global.h"

class QMessageLogContext;

namespace QtLogger {

using SinkPtr = QSharedPointer<class Sink>;

class QTLOGGER_EXPORT Sink : public Handler
{
public:
    virtual void send(const LogMessage &logMsg) = 0;
    virtual bool flush() { return true; }

    HandlerType type() const override { return HandlerType::Sink; }

    bool process(LogMessage &logMsg) override final
    {
        send(logMsg);
        return true;
    }
};

} // namespace QtLogger

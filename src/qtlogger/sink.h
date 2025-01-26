// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "handler.h"
#include "logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT Sink : public Handler
{
public:
    virtual void send(const LogMessage &lmsg) = 0;
    virtual bool flush() { return true; }

    HandlerType type() const override final { return HandlerType::Sink; }

    bool process(LogMessage &lmsg) override final
    {
        send(lmsg);
        return true;
    }
};

using SinkPtr = QSharedPointer<Sink>;

} // namespace QtLogger

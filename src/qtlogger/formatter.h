// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "handler.h"
#include "logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT Formatter : public Handler
{
public:
    virtual ~Formatter() = default;

    virtual QString format(const LogMessage &lmsg) = 0;

    HandlerType type() const override { return HandlerType::Formatter; }

    bool process(LogMessage &lmsg) override final
    {
        lmsg.setFormattedMessage(format(lmsg));
        return true;
    }
};

using FormatterPtr = QSharedPointer<Formatter>;

} // namespace QtLogger

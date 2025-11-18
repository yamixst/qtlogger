// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QSharedPointer>

#include "handler.h"
#include "logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT Filter : public Handler
{
public:
    virtual ~Filter() = default;

    virtual bool filter(const LogMessage &lmsg) = 0;

    HandlerType type() const override final { return HandlerType::Filter; }

    bool process(LogMessage &lmsg) override final { return filter(lmsg); }
};

using FilterPtr = QSharedPointer<Filter>;

} // namespace QtLogger

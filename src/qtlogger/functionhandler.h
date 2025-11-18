// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

#include <QSharedPointer>

#include "handler.h"
#include "logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT FunctionHandler : public Handler
{
public:
    using Function = std::function<bool(LogMessage &)>;

    FunctionHandler(Function function) : m_function(std::move(function)) { }

    bool process(LogMessage &lmsg) override { return m_function(lmsg); }

private:
    Function m_function;
};

using FunctionHandlerPtr = QSharedPointer<FunctionHandler>;

} // namespace QtLogger

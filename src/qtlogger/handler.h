// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "logger_global.h"
#include "logmessage.h"

namespace QtLogger {

class QTLOGGER_EXPORT Handler
{
public:
    enum class HandlerType { Handler, AttrHandler, Filter, Formatter, Sink, Pipeline, Mixed };

    virtual ~Handler() = default;

    virtual HandlerType type() const { return HandlerType::Handler; }

    virtual bool process(LogMessage &lmsg) = 0;
};

using HandlerPtr = QSharedPointer<Handler>;

} // namespace QtLogger

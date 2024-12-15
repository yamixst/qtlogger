// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "messagehandler.h"
#include "logger_global.h"

class QMessageLogContext;

namespace QtLogger {

class QTLOGGER_EXPORT Filter : public MessageHandler
{
public:
    virtual ~Filter() = default;

    virtual bool filter(const LogMessage &logMsg) const = 0;

    HandlerType type() const override { return HandlerType::Filter; }

    bool process(LogMessage &logMsg) override final { return filter(logMsg); }
};

using FilterPtr = QSharedPointer<Filter>;

} // namespace QtLogger

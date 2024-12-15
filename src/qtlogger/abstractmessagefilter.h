// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "messagehandler.h"
#include "logger_global.h"

class QMessageLogContext;

namespace QtLogger {

class QTLOGGER_EXPORT AbstractMessageFilter : public MessageHandler
{
public:
    virtual ~AbstractMessageFilter() = default;

    virtual bool filter(const LogMessage &logMsg) const = 0;

    Type type() const override { return MessageHandler::Filter; }

    bool process(LogMessage &logMsg) override final { return filter(logMsg); }
};

using AbstractMessageFilterPtr = QSharedPointer<AbstractMessageFilter>;

} // namespace QtLogger

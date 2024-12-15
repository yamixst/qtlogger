// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "abstractmessageprocessor.h"
#include "logger_global.h"

class QMessageLogContext;

namespace QtLogger {

class QTLOGGER_EXPORT AbstractMessageFormatter : public AbstractMessageProcessor
{
public:
    virtual ~AbstractMessageFormatter() = default;

    virtual QString format(const LogMessage &logMsg) const = 0;

    Type processorType() const override { return AbstractMessageProcessor::Formatter; }

    bool process(LogMessage &logMsg) override final
    {
        logMsg.setFormattedMessage(format(logMsg));
        return true;
    }
};

using AbstractMessageFormatterPtr = QSharedPointer<AbstractMessageFormatter>;

} // namespace QtLogger

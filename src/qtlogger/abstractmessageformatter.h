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

    virtual QString format(const DebugMessage &dmesg) const = 0;

    Type processorType() const override { return AbstractMessageProcessor::Formatter; }

    bool process(DebugMessage &dmesg) override final
    {
        dmesg.setFormattedMessage(format(dmesg));
        return true;
    }
};

using AbstractMessageFormatterPtr = QSharedPointer<AbstractMessageFormatter>;

} // namespace QtLogger

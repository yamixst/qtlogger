// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "abstractmessageprocessor.h"
#include "logger_global.h"

class QMessageLogContext;

namespace QtLogger {

class QTLOGGER_EXPORT AbstractMessageFilter : public AbstractMessageProcessor
{
public:
    virtual ~AbstractMessageFilter() = default;

    virtual bool filter(const DebugMessage &dmesg) const = 0;

    Type processorType() const override { return AbstractMessageProcessor::Filter; }

    bool process(DebugMessage &dmesg) override final { return filter(dmesg); }
};

using AbstractMessageFilterPtr = QSharedPointer<AbstractMessageFilter>;

} // namespace QtLogger

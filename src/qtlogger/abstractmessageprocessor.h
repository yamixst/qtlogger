// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "logmessage.h"
#include "logger_global.h"

class QMessageLogContext;

namespace QtLogger {

class QTLOGGER_EXPORT AbstractMessageProcessor
{
public:
    enum Type { Processor, Filter, Formatter, Sink, Handler };

    virtual ~AbstractMessageProcessor() = default;

    virtual Type processorType() const { return AbstractMessageProcessor::Processor; }

    virtual bool process(LogMessage &logMsg) = 0;
};

using AbstractMessageProcessorPtr = QSharedPointer<AbstractMessageProcessor>;

} // namespace QtLogger

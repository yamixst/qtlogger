// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QList>
#include <QSharedPointer>

#include <initializer_list>

#include "abstractmessagefilter.h"
#include "abstractmessageformatter.h"
#include "abstractmessageprocessor.h"
#include "abstractmessagesink.h"
#include "filters/functionfilter.h"
#include "filters/regexpfilter.h"
#include "formatters/functionformatter.h"
#include "formatters/patternformatter.h"
#include "logger_global.h"

namespace QtLogger {

using PipelineHandlerPtr = QSharedPointer<class PipelineHandler>;

class QTLOGGER_EXPORT PipelineHandler : public AbstractMessageProcessor
{
public:
    PipelineHandler();
    PipelineHandler(std::initializer_list<AbstractMessageProcessorPtr> processors);

    Type processorType() const override { return AbstractMessageProcessor::Handler; }

    void append(const AbstractMessageProcessorPtr &processor);
    void append(std::initializer_list<AbstractMessageProcessorPtr> processors);
    void insertAfter(Type type, const AbstractMessageProcessorPtr &processor);
    void insertAfter(Type type, Type typeRight, const AbstractMessageProcessorPtr &processor);
    void remove(const AbstractMessageProcessorPtr &processor);
    void clear();

    void clear(Type type);

    void appendFilter(const AbstractMessageFilterPtr &filter);
    FunctionFilterPtr appendFilter(const std::function<bool(const LogMessage &)> &function);
    RegExpFilterPtr appendFilter(const QRegularExpression &regExp);
    void clearFilters();

    void setFormatter(const AbstractMessageFormatterPtr &formatter);
    FunctionFormatterPtr setFormatter(const std::function<QString(const LogMessage &)> &function);
    PatternFormatterPtr setFormatter(const QString &pattern);
    void clearFormatters();

    void appendSink(const AbstractMessageSinkPtr &sink);
    void clearSinks();

    void appendHandler(const PipelineHandlerPtr &handler);
    void clearHandlers();

    PipelineHandler &operator<<(const AbstractMessageProcessorPtr &processor);

    bool process(LogMessage &logMsg) override;

    void flush();

private:
    QList<AbstractMessageProcessorPtr> m_processors;
};

inline PipelineHandler &operator<<(PipelineHandler *handler,
                                  const AbstractMessageProcessorPtr &processor)
{
    return *handler << processor;
}

inline PipelineHandler &operator<<(PipelineHandlerPtr handler,
                                  const AbstractMessageProcessorPtr &processor)
{
    return *handler << processor;
}

} // namespace QtLogger

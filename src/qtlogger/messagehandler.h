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

using MessageHandlerPtr = QSharedPointer<class MessageHandler>;

class QTLOGGER_EXPORT MessageHandler : public AbstractMessageProcessor
{
public:
    MessageHandler();
    MessageHandler(std::initializer_list<AbstractMessageProcessorPtr> processors);

    Type processorType() const override { return AbstractMessageProcessor::Handler; }

    void append(const AbstractMessageProcessorPtr &processor);
    void append(std::initializer_list<AbstractMessageProcessorPtr> processors);
    void insertAfter(Type type, const AbstractMessageProcessorPtr &processor);
    void insertAfter(Type type, Type typeRight, const AbstractMessageProcessorPtr &processor);
    void remove(const AbstractMessageProcessorPtr &processor);
    void clear();

    void clear(Type type);

    void appendFilter(const AbstractMessageFilterPtr &filter);
    FunctionFilterPtr appendFilter(const std::function<bool(const DebugMessage &)> &function);
    RegExpFilterPtr appendFilter(const QRegularExpression &regExp);
    void clearFilters();

    void setFormatter(const AbstractMessageFormatterPtr &formatter);
    FunctionFormatterPtr setFormatter(const std::function<QString(const DebugMessage &)> &function);
    PatternFormatterPtr setFormatter(const QString &pattern);
    void clearFormatters();

    void appendSink(const AbstractMessageSinkPtr &sink);
    void clearSinks();

    void appendHandler(const MessageHandlerPtr &handler);
    void clearHandlers();

    MessageHandler &operator<<(const AbstractMessageProcessorPtr &processor);

    bool process(DebugMessage &dmesg) override;

    void flush();

private:
    QList<AbstractMessageProcessorPtr> m_processors;
};

inline MessageHandler &operator<<(MessageHandler *handler,
                                  const AbstractMessageProcessorPtr &processor)
{
    return *handler << processor;
}

inline MessageHandler &operator<<(MessageHandlerPtr handler,
                                  const AbstractMessageProcessorPtr &processor)
{
    return *handler << processor;
}

} // namespace QtLogger

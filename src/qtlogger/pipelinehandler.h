// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QList>
#include <QSharedPointer>

#include <initializer_list>

#include "filter.h"
#include "formatter.h"
#include "messagehandler.h"
#include "sink.h"
#include "filters/functionfilter.h"
#include "filters/regexpfilter.h"
#include "formatters/functionformatter.h"
#include "formatters/patternformatter.h"
#include "logger_global.h"

namespace QtLogger {

using PipelineHandlerPtr = QSharedPointer<class PipelineHandler>;

class QTLOGGER_EXPORT PipelineHandler : public MessageHandler
{
public:
    PipelineHandler();
    PipelineHandler(std::initializer_list<MessageHandlerPtr> handlers);

    HandlerType type() const override { return HandlerType::Pipeline; }

    void append(const MessageHandlerPtr &handler);
    void append(std::initializer_list<MessageHandlerPtr> handlers);
    void insertAfter(HandlerType type, const MessageHandlerPtr &handler);
    void insertAfter(HandlerType type, HandlerType typeRight, const MessageHandlerPtr &handler);
    void remove(const MessageHandlerPtr &handler);
    void clear();

    void clear(HandlerType type);

    void appendFilter(const FilterPtr &filter);
    FunctionFilterPtr appendFilter(const std::function<bool(const LogMessage &)> &function);
    RegExpFilterPtr appendFilter(const QRegularExpression &regExp);
    void clearFilters();

    void setFormatter(const FormatterPtr &formatter);
    FunctionFormatterPtr setFormatter(const std::function<QString(const LogMessage &)> &function);
    PatternFormatterPtr setFormatter(const QString &pattern);
    void clearFormatters();

    void appendSink(const SinkPtr &sink);
    void clearSinks();

    void appendHandler(const PipelineHandlerPtr &pipeline);
    void clearHandlers();

    PipelineHandler &operator<<(const MessageHandlerPtr &handler);

    bool process(LogMessage &logMsg) override;

    void flush();

private:
    QList<MessageHandlerPtr> m_handlers;
};

inline PipelineHandler &operator<<(PipelineHandler *pipeline,
                                  const MessageHandlerPtr &handler)
{
    return *pipeline << handler;
}

inline PipelineHandler &operator<<(PipelineHandlerPtr pipeline,
                                  const MessageHandlerPtr &handler)
{
    return *pipeline << handler;
}

} // namespace QtLogger

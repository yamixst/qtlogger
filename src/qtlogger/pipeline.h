// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QList>
#include <QSharedPointer>

#include <initializer_list>

#include "filter.h"
#include "formatter.h"
#include "handler.h"
#include "sink.h"
#include "filters/functionfilter.h"
#include "filters/regexpfilter.h"
#include "formatters/functionformatter.h"
#include "formatters/patternformatter.h"
#include "logger_global.h"

namespace QtLogger {

using PipelinePtr = QSharedPointer<class Pipeline>;

class QTLOGGER_EXPORT Pipeline : public Handler
{
public:
    Pipeline();
    Pipeline(std::initializer_list<HandlerPtr> handlers);

    HandlerType type() const override { return HandlerType::Pipeline; }

    void append(const HandlerPtr &handler);
    void append(std::initializer_list<HandlerPtr> handlers);
    void insertAfter(HandlerType type, const HandlerPtr &handler);
    void insertAfter(HandlerType type, HandlerType typeRight, const HandlerPtr &handler);
    void remove(const HandlerPtr &handler);
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

    void appendHandler(const PipelinePtr &pipeline);
    void clearHandlers();

    Pipeline &operator<<(const HandlerPtr &handler);

    bool process(LogMessage &logMsg) override;

    void flush();

private:
    QList<HandlerPtr> m_handlers;
};

inline Pipeline &operator<<(Pipeline *pipeline,
                                  const HandlerPtr &handler)
{
    return *pipeline << handler;
}

inline Pipeline &operator<<(PipelinePtr pipeline,
                                  const HandlerPtr &handler)
{
    return *pipeline << handler;
}

} // namespace QtLogger

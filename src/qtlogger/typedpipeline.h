#pragma once

#include "filter.h"
#include "filters/functionfilter.h"
#include "filters/regexpfilter.h"
#include "formatter.h"
#include "formatters/functionformatter.h"
#include "formatters/patternformatter.h"
#include "handler.h"
#include "logger_global.h"
#include "pipeline.h"
#include "sink.h"

namespace QtLogger {

class QTLOGGER_EXPORT TypedPipeline : public Pipeline
{
public:
    explicit TypedPipeline(bool scoped = false) : Pipeline(scoped) {}
    ~TypedPipeline() override;

    void insertBefore(HandlerType type, const HandlerPtr &handler);
    void insertAfter(HandlerType type, const HandlerPtr &handler);
    void insertBetween(HandlerType leftType, HandlerType rightType, const HandlerPtr &handler);
    void clearType(HandlerType type);

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

    void appendPipeline(const PipelinePtr &pipeline);
    void clearPipelines();

    void flush();
};

using TypedPipelinePtr = QSharedPointer<TypedPipeline>;

} // namespace QtLogger

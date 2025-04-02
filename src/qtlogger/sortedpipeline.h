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

class QTLOGGER_EXPORT SortedPipeline : public Pipeline
{
public:
    explicit SortedPipeline(bool scoped = false) : Pipeline(scoped) {}

    void insertBefore(HandlerType type, const HandlerPtr &handler);
    void insertAfter(HandlerType type, const HandlerPtr &handler);
    void insertBetween(HandlerType leftType, HandlerType rightType, const HandlerPtr &handler);
    void clearType(HandlerType type);

    void appendFilter(const FilterPtr &filter);
    void clearFilters();

    void setFormatter(const FormatterPtr &formatter);
    void clearFormatters();

    void appendSink(const SinkPtr &sink);
    void clearSinks();

    void appendPipeline(const PipelinePtr &pipeline);
    void clearPipelines();
};

using SortedPipelinePtr = QSharedPointer<SortedPipeline>;

} // namespace QtLogger

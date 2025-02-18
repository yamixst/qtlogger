#pragma once

#include <QSet>

#include "attrhandler.h"
#include "filter.h"
#include "formatter.h"
#include "handler.h"
#include "logger_global.h"
#include "pipeline.h"
#include "sink.h"

namespace QtLogger {

class QTLOGGER_EXPORT SortedPipeline : public Pipeline
{
public:
    explicit SortedPipeline(bool scoped = false) : Pipeline(scoped) { }

    void insertBetweenNearLeft(const QSet<HandlerType> &leftType,
                               const QSet<HandlerType> &rightType, const HandlerPtr &handler);
    void insertBetweenNearRight(const QSet<HandlerType> &leftType,
                                const QSet<HandlerType> &rightType, const HandlerPtr &handler);
    void clear(HandlerType type);
    void clear();

    void appendAttrHandler(const AttrHandlerPtr &attrHandler);
    void clearAttrHandlers();

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

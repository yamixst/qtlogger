#include "sortedpipeline.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
void SortedPipeline::insertBefore(HandlerType type, const HandlerPtr &handler)
{
    auto first = std::find_if(handlers().begin(), handlers().end(),
                              [type](const HandlerPtr &x) { return x->type() == type; });

    if (first == handlers().end()) {
        handlers().prepend(handler);
        return;
    }

    handlers().insert(first, handler);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::insertAfter(HandlerType type, const HandlerPtr &handler)
{
    auto first = std::find_if(handlers().begin(), handlers().end(),
                              [type](const HandlerPtr &x) { return x->type() == type; });

    if (first == handlers().end()) {
        handlers().prepend(handler);
        return;
    }

    auto last = std::find_if(first, handlers().end(),
                             [type](const HandlerPtr &x) { return x->type() != type; });

    handlers().insert(last, handler);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::insertBetween(HandlerType leftType, HandlerType rightType,
                                  const HandlerPtr &handler)
{
    auto firstLeft =
            std::find_if(handlers().begin(), handlers().end(),
                         [leftType](const HandlerPtr &x) { return x->type() == leftType; });

    if (firstLeft == handlers().end()) {

        auto firstRight =
                std::find_if(handlers().begin(), handlers().end(),
                             [rightType](const auto &x) { return x->type() == rightType; });

        if (firstRight != handlers().end()) {
            handlers().insert(firstRight, handler);
        } else {
            handlers().prepend(handler);
        }

        return;
    }

    auto lastLeft = std::find_if(firstLeft, handlers().end(),
                                 [leftType](const HandlerPtr &x) { return x->type() != leftType; });

    handlers().insert(lastLeft, handler);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::clear(HandlerType type)
{
    QMutableListIterator<HandlerPtr> iter(handlers());

    while (iter.hasNext()) {
        if (iter.next()->type() == type) {
            iter.remove();
        }
    }
}

QTLOGGER_DECL_SPEC
void SortedPipeline::clear()
{
    Pipeline::clear();
}

QTLOGGER_DECL_SPEC
void SortedPipeline::appendFilter(const FilterPtr &filter)
{
    if (filter.isNull())
        return;

    insertAfter(HandlerType::Filter, filter);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::clearFilters()
{
    clear(HandlerType::Filter);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::setFormatter(const FormatterPtr &formatter)
{
    if (formatter.isNull())
        return;

    clearFormatters();

    insertAfter(HandlerType::Filter, formatter);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::clearFormatters()
{
    clear(HandlerType::Formatter);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::appendSink(const SinkPtr &sink)
{
    append(sink);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::clearSinks()
{
    clear(HandlerType::Sink);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::appendPipeline(const PipelinePtr &pipeline)
{
    append(pipeline);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::clearPipelines()
{
    clear(HandlerType::Pipeline);
}

} // namespace QtLogger

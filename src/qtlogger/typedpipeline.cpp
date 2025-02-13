#include "typedpipeline.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
TypedPipeline::~TypedPipeline()
{
    flush();
}

QTLOGGER_DECL_SPEC
void TypedPipeline::insertBefore(HandlerType type, const HandlerPtr &handler)
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
void TypedPipeline::insertAfter(HandlerType type, const HandlerPtr &handler)
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
void TypedPipeline::insertBetween(HandlerType leftType, HandlerType rightType,
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
void TypedPipeline::clearType(HandlerType type)
{
    QMutableListIterator<HandlerPtr> iter(handlers());

    while (iter.hasNext()) {
        if (iter.next()->type() == type) {
            iter.remove();
        }
    }
}

QTLOGGER_DECL_SPEC
void TypedPipeline::appendFilter(const FilterPtr &filter)
{
    if (filter.isNull())
        return;

    insertAfter(HandlerType::Filter, filter);
}

QTLOGGER_DECL_SPEC
void TypedPipeline::clearFilters()
{
    clearType(HandlerType::Filter);
}

QTLOGGER_DECL_SPEC
void TypedPipeline::setFormatter(const FormatterPtr &formatter)
{
    if (formatter.isNull())
        return;

    clearFormatters();

    insertAfter(HandlerType::Filter, formatter);
}

QTLOGGER_DECL_SPEC
void TypedPipeline::clearFormatters()
{
    clearType(HandlerType::Formatter);
}

QTLOGGER_DECL_SPEC
void TypedPipeline::appendSink(const SinkPtr &sink)
{
    append(sink);
}

QTLOGGER_DECL_SPEC
void TypedPipeline::clearSinks()
{
    clearType(HandlerType::Sink);
}

QTLOGGER_DECL_SPEC
void TypedPipeline::appendPipeline(const PipelinePtr &pipeline)
{
    append(pipeline);
}

QTLOGGER_DECL_SPEC
void TypedPipeline::clearPipelines()
{
    clearType(HandlerType::Pipeline);
}

QTLOGGER_DECL_SPEC
void TypedPipeline::flush()
{
    for (auto &handler : handlers()) {
        switch (handler->type()) {
        case HandlerType::Sink: {
            auto sink = handler.dynamicCast<Sink>();
            if (sink)
                sink->flush();
            break;
        }
        case HandlerType::Pipeline: {
            auto pipeline = handler.dynamicCast<TypedPipeline>();
            if (pipeline)
                pipeline->flush();
            break;
        }
        default:
            break;
        }
    }
}

} // namespace QtLogger

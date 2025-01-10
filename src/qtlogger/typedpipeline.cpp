#include "typedpipeline.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
TypedPipeline::~TypedPipeline()
{
    flush();
}

QTLOGGER_DECL_SPEC
void TypedPipeline::insertAfter(HandlerType type, const HandlerPtr &handler)
{
    auto first = std::find_if(handlers().cbegin(), handlers().cend(),
                              [type](const HandlerPtr &x) { return x->type() == type; });

    if (first == handlers().cend()) {
        handlers().prepend(handler);
        return;
    }

    auto last = std::find_if(first, handlers().cend(),
                             [type](const HandlerPtr &x) { return x->type() != type; });

    handlers().insert(last, handler);
}

QTLOGGER_DECL_SPEC
void TypedPipeline::insertBetween(HandlerType leftType, HandlerType rightType,
                                  const HandlerPtr &handler)
{
    auto firstLeft =
            std::find_if(handlers().cbegin(), handlers().cend(),
                         [leftType](const HandlerPtr &x) { return x->type() == leftType; });

    if (firstLeft == handlers().cend()) {

        auto firstRight =
                std::find_if(handlers().cbegin(), handlers().cend(),
                             [rightType](const auto &x) { return x->type() == rightType; });

        if (firstRight != handlers().cend()) {
            handlers().insert(firstRight, handler);
        } else {
            handlers().prepend(handler);
        }

        return;
    }

    auto lastLeft = std::find_if(firstLeft, handlers().cend(),
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
FunctionFilterPtr
TypedPipeline::appendFilter(const std::function<bool(const LogMessage &)> &function)
{
    const auto f = FunctionFilterPtr::create(function);

    appendFilter(f);

    return f;
}

QTLOGGER_DECL_SPEC
RegExpFilterPtr TypedPipeline::appendFilter(const QRegularExpression &regExp)
{
    const auto f = RegExpFilterPtr::create(regExp);

    appendFilter(f);

    return f;
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
FunctionFormatterPtr TypedPipeline::setFormatter(const FunctionFormatter::Function &function)
{
    const auto f = FunctionFormatterPtr::create(function);

    setFormatter(f);

    return f;
}

QTLOGGER_DECL_SPEC
PatternFormatterPtr TypedPipeline::setFormatter(const QString &pattern)
{
    const auto f = PatternFormatterPtr::create(pattern);

    setFormatter(f);

    return f;
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

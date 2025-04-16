#include "sortedpipeline.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
void SortedPipeline::insertBetweenNearLeft(const QSet<HandlerType> &leftType,
                                           const QSet<HandlerType> &rightType,
                                           const HandlerPtr &handler)
{
    auto firstRight =
            std::find_if(handlers().begin(), handlers().end(),
                         [&rightType](const auto &x) { return rightType.contains(x->type()); });

    auto lastLeft = std::find_if(firstRight, handlers().begin(), [&leftType](const HandlerPtr &x) {
        return leftType.contains(x->type());
    });

    handlers().insert(lastLeft, handler);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::insertBetweenNearRight(const QSet<HandlerType> &leftType,
                                            const QSet<HandlerType> &rightType,
                                            const HandlerPtr &handler)
{
    auto lastLeft =
            std::find_if(handlers().end(), handlers().begin(),
                         [&leftType](const HandlerPtr &x) { return leftType.contains(x->type()); });

    auto firstRight = std::find_if(lastLeft, handlers().end(), [&rightType](const auto &x) {
        return rightType.contains(x->type());
    });

    handlers().insert(firstRight, handler);
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
void SortedPipeline::appendAttrHandler(const AttrHandlerPtr &attrHandler)
{
    if (attrHandler.isNull())
        return;

    insertBetweenNearLeft({ HandlerType::AttrHandler },
                          { HandlerType::Filter, HandlerType::Formatter, HandlerType::Sink },
                          attrHandler);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::clearAttrHandlers()
{
    clear(HandlerType::AttrHandler);
}

QTLOGGER_DECL_SPEC
void SortedPipeline::appendFilter(const FilterPtr &filter)
{
    if (filter.isNull())
        return;

    insertBetweenNearLeft({ HandlerType::AttrHandler, HandlerType::Filter },
                          { HandlerType::Formatter, HandlerType::Sink }, filter);
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

    insertBetweenNearRight({ HandlerType::AttrHandler, HandlerType::Filter }, { HandlerType::Sink },
                           formatter);
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

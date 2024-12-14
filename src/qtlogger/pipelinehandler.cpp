// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "pipelinehandler.h"

#include "abstractmessagesink.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
PipelineHandler::PipelineHandler() { }

QTLOGGER_DECL_SPEC
PipelineHandler::PipelineHandler(std::initializer_list<AbstractMessageProcessorPtr> processors)
    : m_processors(processors)
{
}

QTLOGGER_DECL_SPEC
void PipelineHandler::append(const AbstractMessageProcessorPtr &processor)
{
    if (processor.isNull())
        return;

    m_processors.append(processor);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::append(std::initializer_list<AbstractMessageProcessorPtr> processors)
{
    m_processors.append(processors);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::insertAfter(Type type, const AbstractMessageProcessorPtr &processor)
{
    auto first = std::find_if(
            m_processors.begin(), m_processors.end(),
            [type](const AbstractMessageProcessorPtr &x) { return x->processorType() == type; });

    if (first == m_processors.end()) {
        m_processors.prepend(processor);
        return;
    }

    auto last =
            std::find_if(first, m_processors.end(), [type](const AbstractMessageProcessorPtr &x) {
                return x->processorType() != type;
            });

    m_processors.insert(last, processor);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::insertAfter(Type type, Type typeRight,
                                 const AbstractMessageProcessorPtr &processor)
{
    auto first = std::find_if(
            m_processors.begin(), m_processors.end(),
            [type](const AbstractMessageProcessorPtr &x) { return x->processorType() == type; });

    if (first == m_processors.end()) {

        auto right = std::find_if(first, m_processors.end(), [typeRight](const auto &x) {
            return x->processorType() == typeRight;
        });

        if (first != m_processors.end()) {
            m_processors.insert(--right, processor);
        } else {
            m_processors.prepend(processor);
        }

        return;
    }

    auto last =
            std::find_if(first, m_processors.end(), [type](const AbstractMessageProcessorPtr &x) {
                return x->processorType() != type;
            });

    m_processors.insert(last, processor);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::remove(const AbstractMessageProcessorPtr &processor)
{
    if (processor.isNull())
        return;

    m_processors.removeAll(processor);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::clear()
{
    m_processors.clear();
}

QTLOGGER_DECL_SPEC
void PipelineHandler::clear(Type type)
{
    QMutableListIterator<AbstractMessageProcessorPtr> iter(m_processors);

    while (iter.hasNext()) {
        if (iter.next()->processorType() == type) {
            iter.remove();
        }
    }
}

QTLOGGER_DECL_SPEC
void PipelineHandler::appendFilter(const AbstractMessageFilterPtr &filter)
{
    if (filter.isNull())
        return;

    auto index = std::find_if(m_processors.begin(), m_processors.end(), [](const auto &x) {
        return x->processorType() != AbstractMessageProcessor::Filter;
    });

    m_processors.insert(index, filter);
}

QTLOGGER_DECL_SPEC
FunctionFilterPtr
PipelineHandler::appendFilter(const std::function<bool(const LogMessage &)> &function)
{
    const auto f = FunctionFilterPtr::create(function);

    appendFilter(f);

    return f;
}

QTLOGGER_DECL_SPEC
RegExpFilterPtr PipelineHandler::appendFilter(const QRegularExpression &regExp)
{
    const auto f = RegExpFilterPtr::create(regExp);

    appendFilter(f);

    return f;
}

QTLOGGER_DECL_SPEC
void PipelineHandler::clearFilters()
{
    clear(Filter);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::setFormatter(const AbstractMessageFormatterPtr &formatter)
{
    if (formatter.isNull())
        return;

    clearFormatters();

    auto index = std::find_if(m_processors.begin(), m_processors.end(), [](const auto &x) {
        return x->processorType() != AbstractMessageProcessor::Filter;
    });

    m_processors.insert(index, formatter);
}

QTLOGGER_DECL_SPEC
FunctionFormatterPtr PipelineHandler::setFormatter(const FunctionFormatter::Function &function)
{
    const auto f = FunctionFormatterPtr::create(function);

    setFormatter(f);

    return f;
}

QTLOGGER_DECL_SPEC
PatternFormatterPtr PipelineHandler::setFormatter(const QString &pattern)
{
    const auto f = PatternFormatterPtr::create(pattern);

    setFormatter(f);

    return f;
}

QTLOGGER_DECL_SPEC
void PipelineHandler::clearFormatters()
{
    clear(AbstractMessageProcessor::Formatter);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::appendSink(const AbstractMessageSinkPtr &sink)
{
    append(sink);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::clearSinks()
{
    clear(AbstractMessageProcessor::Sink);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::appendHandler(const PipelineHandlerPtr &handler)
{
    append(handler);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::clearHandlers()
{
    clear(AbstractMessageProcessor::Handler);
}

QTLOGGER_DECL_SPEC
PipelineHandler &PipelineHandler::operator<<(const AbstractMessageProcessorPtr &processor)
{
    append(processor);
    return *this;
}

QTLOGGER_DECL_SPEC
bool PipelineHandler::process(LogMessage &logMsg)
{
    for (auto &processor : m_processors) {
        if (!processor)
            continue;
        if (!processor->process(logMsg))
            break;
    }

    return true;
}

QTLOGGER_DECL_SPEC
void PipelineHandler::flush()
{
    for (auto &processor : m_processors) {
        switch (processor->processorType()) {
        case AbstractMessageProcessor::Sink: {
            auto sink = processor.dynamicCast<AbstractMessageSink>();
            if (sink)
                sink->flush();
            break;
        }
        case AbstractMessageProcessor::Handler: {
            auto handler = processor.dynamicCast<PipelineHandler>();
            if (handler)
                handler->flush();
            break;
        }
        default:
            break;
        }
    }
}

} // namespace QtLogger

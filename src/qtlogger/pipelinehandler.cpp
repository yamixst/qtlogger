// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "pipelinehandler.h"

#include "abstractmessagesink.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
PipelineHandler::PipelineHandler() { }

QTLOGGER_DECL_SPEC
PipelineHandler::PipelineHandler(std::initializer_list<MessageHandlerPtr> handlers)
    : m_handlers(handlers)
{
}

QTLOGGER_DECL_SPEC
void PipelineHandler::append(const MessageHandlerPtr &handler)
{
    if (handler.isNull())
        return;

    m_handlers.append(handler);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::append(std::initializer_list<MessageHandlerPtr> handlers)
{
    m_handlers.append(handlers);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::insertAfter(Type type, const MessageHandlerPtr &handler)
{
    auto first = std::find_if(
            m_handlers.begin(), m_handlers.end(),
            [type](const MessageHandlerPtr &x) { return x->type() == type; });

    if (first == m_handlers.end()) {
        m_handlers.prepend(handler);
        return;
    }

    auto last =
            std::find_if(first, m_handlers.end(), [type](const MessageHandlerPtr &x) {
                return x->type() != type;
            });

    m_handlers.insert(last, handler);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::insertAfter(Type type, Type typeRight,
                                 const MessageHandlerPtr &handler)
{
    auto first = std::find_if(
            m_handlers.begin(), m_handlers.end(),
            [type](const MessageHandlerPtr &x) { return x->type() == type; });

    if (first == m_handlers.end()) {

        auto right = std::find_if(first, m_handlers.end(), [typeRight](const auto &x) {
            return x->type() == typeRight;
        });

        if (first != m_handlers.end()) {
            m_handlers.insert(--right, handler);
        } else {
            m_handlers.prepend(handler);
        }

        return;
    }

    auto last =
            std::find_if(first, m_handlers.end(), [type](const MessageHandlerPtr &x) {
                return x->type() != type;
            });

    m_handlers.insert(last, handler);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::remove(const MessageHandlerPtr &handler)
{
    if (handler.isNull())
        return;

    m_handlers.removeAll(handler);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::clear()
{
    m_handlers.clear();
}

QTLOGGER_DECL_SPEC
void PipelineHandler::clear(Type type)
{
    QMutableListIterator<MessageHandlerPtr> iter(m_handlers);

    while (iter.hasNext()) {
        if (iter.next()->type() == type) {
            iter.remove();
        }
    }
}

QTLOGGER_DECL_SPEC
void PipelineHandler::appendFilter(const AbstractMessageFilterPtr &filter)
{
    if (filter.isNull())
        return;

    auto index = std::find_if(m_handlers.begin(), m_handlers.end(), [](const auto &x) {
        return x->type() != MessageHandler::Filter;
    });

    m_handlers.insert(index, filter);
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

    auto index = std::find_if(m_handlers.begin(), m_handlers.end(), [](const auto &x) {
        return x->type() != MessageHandler::Filter;
    });

    m_handlers.insert(index, formatter);
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
    clear(MessageHandler::Formatter);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::appendSink(const AbstractMessageSinkPtr &sink)
{
    append(sink);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::clearSinks()
{
    clear(MessageHandler::Sink);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::appendHandler(const PipelineHandlerPtr &pipeline)
{
    append(pipeline);
}

QTLOGGER_DECL_SPEC
void PipelineHandler::clearHandlers()
{
    clear(MessageHandler::Pipeline);
}

QTLOGGER_DECL_SPEC
PipelineHandler &PipelineHandler::operator<<(const MessageHandlerPtr &handler)
{
    append(handler);
    return *this;
}

QTLOGGER_DECL_SPEC
bool PipelineHandler::process(LogMessage &logMsg)
{
    for (auto &handler : m_handlers) {
        if (!handler)
            continue;
        if (!handler->process(logMsg))
            break;
    }

    return true;
}

QTLOGGER_DECL_SPEC
void PipelineHandler::flush()
{
    for (auto &handler : m_handlers) {
        switch (handler->type()) {
        case MessageHandler::Sink: {
            auto sink = handler.dynamicCast<AbstractMessageSink>();
            if (sink)
                sink->flush();
            break;
        }
        case MessageHandler::Pipeline: {
            auto pipeline = handler.dynamicCast<PipelineHandler>();
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

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "pipeline.h"

#include "sink.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
Pipeline::Pipeline() { }

QTLOGGER_DECL_SPEC
Pipeline::Pipeline(std::initializer_list<MessageHandlerPtr> handlers)
    : m_handlers(handlers)
{
}

QTLOGGER_DECL_SPEC
void Pipeline::append(const MessageHandlerPtr &handler)
{
    if (handler.isNull())
        return;

    m_handlers.append(handler);
}

QTLOGGER_DECL_SPEC
void Pipeline::append(std::initializer_list<MessageHandlerPtr> handlers)
{
    m_handlers.append(handlers);
}

QTLOGGER_DECL_SPEC
void Pipeline::insertAfter(HandlerType type, const MessageHandlerPtr &handler)
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
void Pipeline::insertAfter(HandlerType type, HandlerType typeRight,
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
void Pipeline::remove(const MessageHandlerPtr &handler)
{
    if (handler.isNull())
        return;

    m_handlers.removeAll(handler);
}

QTLOGGER_DECL_SPEC
void Pipeline::clear()
{
    m_handlers.clear();
}

QTLOGGER_DECL_SPEC
void Pipeline::clear(HandlerType type)
{
    QMutableListIterator<MessageHandlerPtr> iter(m_handlers);

    while (iter.hasNext()) {
        if (iter.next()->type() == type) {
            iter.remove();
        }
    }
}

QTLOGGER_DECL_SPEC
void Pipeline::appendFilter(const FilterPtr &filter)
{
    if (filter.isNull())
        return;

    auto index = std::find_if(m_handlers.begin(), m_handlers.end(), [](const auto &x) {
        return x->type() != HandlerType::Filter;
    });

    m_handlers.insert(index, filter);
}

QTLOGGER_DECL_SPEC
FunctionFilterPtr
Pipeline::appendFilter(const std::function<bool(const LogMessage &)> &function)
{
    const auto f = FunctionFilterPtr::create(function);

    appendFilter(f);

    return f;
}

QTLOGGER_DECL_SPEC
RegExpFilterPtr Pipeline::appendFilter(const QRegularExpression &regExp)
{
    const auto f = RegExpFilterPtr::create(regExp);

    appendFilter(f);

    return f;
}

QTLOGGER_DECL_SPEC
void Pipeline::clearFilters()
{
    clear(HandlerType::Filter);
}

QTLOGGER_DECL_SPEC
void Pipeline::setFormatter(const FormatterPtr &formatter)
{
    if (formatter.isNull())
        return;

    clearFormatters();

    auto index = std::find_if(m_handlers.begin(), m_handlers.end(), [](const auto &x) {
        return x->type() != HandlerType::Filter;
    });

    m_handlers.insert(index, formatter);
}

QTLOGGER_DECL_SPEC
FunctionFormatterPtr Pipeline::setFormatter(const FunctionFormatter::Function &function)
{
    const auto f = FunctionFormatterPtr::create(function);

    setFormatter(f);

    return f;
}

QTLOGGER_DECL_SPEC
PatternFormatterPtr Pipeline::setFormatter(const QString &pattern)
{
    const auto f = PatternFormatterPtr::create(pattern);

    setFormatter(f);

    return f;
}

QTLOGGER_DECL_SPEC
void Pipeline::clearFormatters()
{
    clear(HandlerType::Formatter);
}

QTLOGGER_DECL_SPEC
void Pipeline::appendSink(const SinkPtr &sink)
{
    append(sink);
}

QTLOGGER_DECL_SPEC
void Pipeline::clearSinks()
{
    clear(HandlerType::Sink);
}

QTLOGGER_DECL_SPEC
void Pipeline::appendHandler(const PipelinePtr &pipeline)
{
    append(pipeline);
}

QTLOGGER_DECL_SPEC
void Pipeline::clearHandlers()
{
    clear(HandlerType::Pipeline);
}

QTLOGGER_DECL_SPEC
Pipeline &Pipeline::operator<<(const MessageHandlerPtr &handler)
{
    append(handler);
    return *this;
}

QTLOGGER_DECL_SPEC
bool Pipeline::process(LogMessage &logMsg)
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
void Pipeline::flush()
{
    for (auto &handler : m_handlers) {
        switch (handler->type()) {
        case HandlerType::Sink: {
            auto sink = handler.dynamicCast<Sink>();
            if (sink)
                sink->flush();
            break;
        }
        case HandlerType::Pipeline: {
            auto pipeline = handler.dynamicCast<Pipeline>();
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

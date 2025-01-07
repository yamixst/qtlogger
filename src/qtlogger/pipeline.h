// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <initializer_list>

#include <QList>
#include <QSharedPointer>

#include "handler.h"
#include "logger_global.h"

namespace QtLogger {

using PipelinePtr = QSharedPointer<class Pipeline>;

class QTLOGGER_EXPORT Pipeline : public Handler
{
public:
    Pipeline() = default;
    Pipeline(std::initializer_list<HandlerPtr> handlers);

    HandlerType type() const override { return HandlerType::Pipeline; }

    void append(const HandlerPtr &handler);
    void append(std::initializer_list<HandlerPtr> handlers);
    void remove(const HandlerPtr &handler);
    void clear();

    Pipeline &operator<<(const HandlerPtr &handler);

    bool process(LogMessage &logMsg) override;

protected:
    QList<HandlerPtr> &handlers() { return m_handlers; }

private:
    QList<HandlerPtr> m_handlers;
};

inline Pipeline &operator<<(Pipeline *pipeline, const HandlerPtr &handler)
{
    return *pipeline << handler;
}

inline Pipeline &operator<<(PipelinePtr pipeline, const HandlerPtr &handler)
{
    return *pipeline << handler;
}

} // namespace QtLogger

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <functional>

#include <QSharedPointer>

#include "../abstractmessagefilter.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT FunctionFilter : public AbstractMessageFilter
{
public:
    using Function = std::function<bool(const DebugMessage &)>;

    FunctionFilter(const Function &function);

    bool filter(const DebugMessage &dmesg) const override;

private:
    Function m_function;
};

using FunctionFilterPtr = QSharedPointer<FunctionFilter>;

inline FunctionFilter::FunctionFilter(const Function &function) : m_function(function) { }

inline bool FunctionFilter::filter(const DebugMessage &dmesg) const
{
    return m_function(dmesg);
}

} // namespace QtLogger

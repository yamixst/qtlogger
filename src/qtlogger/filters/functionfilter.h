// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <functional>

#include <QSharedPointer>

#include "../filter.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT FunctionFilter : public Filter
{
public:
    using Function = std::function<bool(const LogMessage &)>;

    FunctionFilter(const Function &function) : m_function(function) { }

    bool filter(const LogMessage &lmsg) override { return m_function(lmsg); }

private:
    Function m_function;
};

using FunctionFilterPtr = QSharedPointer<FunctionFilter>;

} // namespace QtLogger

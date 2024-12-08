// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <functional>

#include <QSharedPointer>

#include "../abstractmessageformatter.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT FunctionFormatter : public AbstractMessageFormatter
{
public:
    using Function = std::function<QString(const DebugMessage &)>;

    FunctionFormatter(const Function &func) : m_func(func) { }

    QString format(const DebugMessage &dmesg) const override { return m_func(dmesg); }

private:
    Function m_func;
};

using FunctionFormatterPtr = QSharedPointer<FunctionFormatter>;

} // namespace QtLogger

// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

#include <QSharedPointer>

#include "../formatter.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT FunctionFormatter : public Formatter
{
public:
    using Function = std::function<QString(const LogMessage &)>;

    FunctionFormatter(const Function &func) : m_func(func) { }

    QString format(const LogMessage &lmsg) override { return m_func(lmsg); }

private:
    Function m_func;
};

using FunctionFormatterPtr = QSharedPointer<FunctionFormatter>;

} // namespace QtLogger

// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QSharedPointer>

#include "../formatter.h"
#include "../logger_global.h"

namespace QtLogger {

using QtLogMessageFormatterPtr = QSharedPointer<class QtLogMessageFormatter>;

class QTLOGGER_EXPORT QtLogMessageFormatter : public Formatter
{
public:
    static QtLogMessageFormatterPtr instance()
    {
        static const auto s_instance = QtLogMessageFormatterPtr(new QtLogMessageFormatter());
        return s_instance;
    }

    QString format(const LogMessage &lmsg) override
    {
        return qFormatLogMessage(lmsg.type(), lmsg.context(), lmsg.message());
    }

private:
    QtLogMessageFormatter() { }
};

} // namespace QtLogger

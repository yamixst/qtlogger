// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "../abstractmessageformatter.h"
#include "../logger_global.h"

namespace QtLogger {

using QtLogMessageFormatterPtr = QSharedPointer<class QtLogMessageFormatter>;

class QTLOGGER_EXPORT QtLogMessageFormatter : public AbstractMessageFormatter
{
public:
    static QtLogMessageFormatterPtr instance()
    {
        static const auto s_instance = QtLogMessageFormatterPtr(new QtLogMessageFormatter());
        return s_instance;
    }

    QString format(const DebugMessage &dmesg) const override
    {
        return qFormatLogMessage(dmesg.type(), dmesg.context(), dmesg.message());
    }

private:
    QtLogMessageFormatter() {}
};

} // namespace QtLogger

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "../abstractmessageformatter.h"
#include "../logger_global.h"

namespace QtLogger {

using NullFormatterPtr = QSharedPointer<class NullFormatter>;

class QTLOGGER_EXPORT NullFormatter : public AbstractMessageFormatter
{
public:
    static NullFormatterPtr instance()
    {
        static const auto s_instance = NullFormatterPtr::create();
        return s_instance;
    }

    QString format(const DebugMessage &dmesg) const override { return dmesg.message(); }
};

} // namespace QtLogger

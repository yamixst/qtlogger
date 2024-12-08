// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "patternformatter.h"

#include "../setmessagepattern.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
PatternFormatter::PatternFormatter(const QString &pattern) : m_pattern(pattern) { }

QTLOGGER_DECL_SPEC
QString PatternFormatter::format(const DebugMessage &dmesg) const
{
    // TODO: write own formatter

    QtLogger::setMessagePattern(m_pattern);

    auto result = qFormatLogMessage(dmesg.type(), dmesg.context(), dmesg.message());

    QtLogger::restorePreviousMessagePattern();

    return result;
}

} // namespace QtLogger

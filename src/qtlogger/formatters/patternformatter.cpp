// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "patternformatter.h"

#include "../utils.h"

namespace QtLogger {

class PatternFormatter::PatternFormatterPrivate
{
public:
    explicit PatternFormatterPrivate(const QString &pattern) : m_pattern(pattern) { }

    QString format(const LogMessage &lmsg)
    {
        // TODO: write own implementation

        QtLogger::setMessagePattern(m_pattern);

        auto result = qFormatLogMessage(lmsg.type(), lmsg.context(), lmsg.message());

        QtLogger::restorePreviousMessagePattern();

        return result;
    }

    QString m_pattern;
};

QTLOGGER_DECL_SPEC
PatternFormatter::PatternFormatter(const QString &pattern)
    : d(new PatternFormatterPrivate(pattern))
{
}

QTLOGGER_DECL_SPEC
PatternFormatter::~PatternFormatter() = default;

QTLOGGER_DECL_SPEC
QString PatternFormatter::format(const LogMessage &lmsg)
{
    return d->format(lmsg);
}

} // namespace QtLogger

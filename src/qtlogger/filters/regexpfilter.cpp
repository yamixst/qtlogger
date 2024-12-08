// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "regexpfilter.h"

#include <QRegularExpression>

namespace QtLogger {

QTLOGGER_DECL_SPEC
RegExpFilter::RegExpFilter(const QRegularExpression &regExp) : m_regExp(regExp) { }

QTLOGGER_DECL_SPEC
QtLogger::RegExpFilter::RegExpFilter(const QString &regExp) : m_regExp(QRegularExpression(regExp))
{
}

QTLOGGER_DECL_SPEC
bool RegExpFilter::filter(const DebugMessage &dmesg) const
{
    return m_regExp.match(dmesg.message()).hasMatch();
}

} // namespace QtLogger

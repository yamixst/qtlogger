// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "setmessagepattern.h"

#include <QString>

#include "messagepatterns.h"

namespace {

QTLOGGER_DECL_SPEC
QString prevMessagePattern(const QString &messagePattern = {})
{
    static QString __prevMessagePattern { QString::fromUtf8(QtLogger::DefaultMessagePattern) };

    if (!messagePattern.isNull())
        __prevMessagePattern = messagePattern;

    return __prevMessagePattern;
}

}

namespace QtLogger {

QTLOGGER_DECL_SPEC
QString setMessagePattern(const QString &messagePattern)
{
    static QString __messagePattern { QString::fromUtf8(QtLogger::DefaultMessagePattern) };

    if (__messagePattern == messagePattern)
        return __messagePattern;

    prevMessagePattern(__messagePattern);

    __messagePattern = messagePattern;

    qSetMessagePattern(__messagePattern);

    return prevMessagePattern();
}

QTLOGGER_DECL_SPEC
QString restorePreviousMessagePattern()
{
    return setMessagePattern(prevMessagePattern());
}

} // namespace QtLogger

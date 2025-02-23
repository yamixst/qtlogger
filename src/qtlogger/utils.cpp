// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "utils.h"

#include <QLoggingCategory>
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
void setFilterRules(const QString &rules)
{
    QLoggingCategory::setFilterRules(
            QString(rules).replace(QChar::fromLatin1(';'), QChar::fromLatin1('\n')));
}

QTLOGGER_DECL_SPEC
QString setMessagePattern(const QString &a_messagePattern)
{
    QString messagePattern = a_messagePattern;

    if (messagePattern.toLower() == QStringLiteral("default")) {
        messagePattern = QString::fromUtf8(DefaultMessagePattern);
    } else if (messagePattern.toLower() == QStringLiteral("pretty")) {
        messagePattern = QString::fromUtf8(PrettyMessagePattern);
    }

    static QString s_messagePattern { QString::fromUtf8(QtLogger::DefaultMessagePattern) };

    if (s_messagePattern == messagePattern)
        return s_messagePattern;

    prevMessagePattern(s_messagePattern);

    s_messagePattern = messagePattern;

    qSetMessagePattern(s_messagePattern);

    return prevMessagePattern();
}

QTLOGGER_DECL_SPEC
QString restorePreviousMessagePattern()
{
    return setMessagePattern(prevMessagePattern());
}

} // namespace QtLogger

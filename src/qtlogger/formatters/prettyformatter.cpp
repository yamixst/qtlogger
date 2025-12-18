// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include "prettyformatter.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
PrettyFormatter::PrettyFormatter(bool colorize, int maxCategoryWidth)
    : m_colorize(colorize), m_maxCategoryWidth(maxCategoryWidth)
{
}

QTLOGGER_DECL_SPEC
QString PrettyFormatter::format(const LogMessage &lmsg)
{
    static const QString timeFormat = QStringLiteral("dd.MM.yyyy hh:mm:ss");
    static const QChar typeLetters[] = {
        QLatin1Char(' '), QLatin1Char('W'), QLatin1Char('E'),
        QLatin1Char('F'), QLatin1Char('I'), QLatin1Char('S')
    };

    // ANSI color codes
    static const QLatin1String reset("\033[0m");
    static const QLatin1String darkGray("\033[90m");
    static const QLatin1String bold("\033[1m");
    static const QLatin1String green("\033[32m");
    static const QLatin1String greenBold("\033[1;32m");
    static const QLatin1String orange("\033[38;5;172m");
    static const QLatin1String darkOrange("\033[38;5;208m");
    static const QLatin1String redBold("\033[1;31m");
    static const QLatin1String darkRedBold("\033[1;38;5;88m");

    static const QLatin1Char space(' ');
    static const QLatin1Char bracketOpen('[');
    static const QLatin1Char bracketClose(']');
    static const QLatin1Char letterT('T');

    const auto type = lmsg.type();
    const auto threadId = lmsg.threadId();
    const auto categoryRaw = lmsg.category();
    const bool isDefaultCategory = (qstrcmp(categoryRaw, "default") == 0);

    // Pre-calculate category string once if needed
    QString category;
    if (!isDefaultCategory) {
        category = QString::fromUtf8(categoryRaw);
    }

    // Estimate result size to minimize reallocations
    // DateTime(19) + space(1) + type(1) + space(1) + thread(~5) + category(~20) + message
    const int estimatedSize = 30 + category.size() + 4 + lmsg.message().size()
                              + (m_colorize ? 80 : 0);

    QString result;
    result.reserve(estimatedSize);

    // DateTime
    result += lmsg.time().toString(timeFormat);
    result += space;

    // Type letter with specific colors
    if (m_colorize) {
        switch (type) {
        case QtInfoMsg:
            result += greenBold;
            result += typeLetters[type];
            result += reset;
            break;
        case QtWarningMsg:
            result += darkOrange;
            result += typeLetters[type];
            result += reset;
            break;
        case QtCriticalMsg:
            result += redBold;
            result += typeLetters[type];
            result += reset;
            break;
        case QtFatalMsg:
            result += darkRedBold;
            result += typeLetters[type];
            result += reset;
            break;
        default:
            result += typeLetters[type];
            break;
        }
    } else {
        result += typeLetters[type];
    }

    result += space;

    // Thread handling with optimized lookup
    auto it = m_threads.find(threadId);
    if (it == m_threads.end()) {
        it = m_threads.insert(threadId, m_threadsIndex++);
    }

    if (m_threads.size() > 1) {
        const int index = it.value();
        if (index == 0) {
            // Calculate width needed for thread field
            int threadWidth = 3; // "T0 " minimum
            if (m_threadsIndex > 10) threadWidth = 4;
            if (m_threadsIndex > 100) threadWidth = 5;
            result += QString(threadWidth, space);
        } else {
            if (m_colorize) {
                result += bold;
            }
            result += letterT;
            result += QString::number(index);
            result += space;
            if (m_colorize) {
                result += reset;
            }
        }
    }

    // Category output (only if not default)
    const int categoryFormatLength = isDefaultCategory ? 0 : (category.size() + 3); // "[name] "
    if (!isDefaultCategory) {
        if (m_colorize) {
            result += darkGray;
        }
        result += bracketOpen;
        result += category;
        result += bracketClose;
        result += space;
        if (m_colorize) {
            result += reset;
        }
    }

    // Space for alignment
    if (m_maxCategoryWidth > 0) {
        if (categoryFormatLength > m_categoryWidth) {
            m_categoryWidth = qMin(categoryFormatLength, m_maxCategoryWidth);
        }
        const int spaceCount = m_categoryWidth - categoryFormatLength;
        if (spaceCount > 0) {
            result += QString(spaceCount, space);
        }
    }

    // Message with color
    if (m_colorize) {
        switch (type) {
        case QtInfoMsg:
            result += green;
            result += lmsg.message();
            result += reset;
            break;
        case QtWarningMsg:
            result += orange;
            result += lmsg.message();
            result += reset;
            break;
        case QtCriticalMsg:
            result += redBold;
            result += lmsg.message();
            result += reset;
            break;
        case QtFatalMsg:
            result += darkRedBold;
            result += lmsg.message();
            result += reset;
            break;
        default:
            result += lmsg.message();
            break;
        }
    } else {
        result += lmsg.message();
    }

    return result;
}

} // namespace QtLogger
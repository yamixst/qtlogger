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
    static const QString time_f { QStringLiteral("dd.MM.yyyy hh:mm:ss") };
    static const QStringList type_l {
        QStringLiteral(" "), QStringLiteral("W"), QStringLiteral("E"),
        QStringLiteral("F"), QStringLiteral("I"), QStringLiteral("S")
    };
    static const QString thread_f { QStringLiteral("T%1 ") };
    static const QString category_f { QStringLiteral("[%1] ") };

    // ANSI color codes
    static const QString reset { QStringLiteral("\033[0m") };
    static const QString darkGray { QStringLiteral("\033[90m") };       // dark gray for category
    static const QString bold { QStringLiteral("\033[1m") };            // bold for thread and datetime
    static const QString green { QStringLiteral("\033[32m") };           // green for Info text
    static const QString greenBold { QStringLiteral("\033[1;32m") };     // bold green for Info letter
    static const QString orange { QStringLiteral("\033[38;5;172m") };   // orange for Warning text
    static const QString darkOrange { QStringLiteral("\033[38;5;208m") }; // dark orange for Warning letter
    static const QString redBold { QStringLiteral("\033[1;31m") };      // red bold for Error
    static const QString darkRedBold { QStringLiteral("\033[1;38;5;88m") }; // dark red/maroon bold for Fatal

    QString result;

    // DateTime - standard text
    result += lmsg.time().toString(time_f);

    result += QChar::fromLatin1(' ');

    // Type letter with specific colors
    auto type = lmsg.type();
    if (m_colorize) {
        switch (type) {
        case QtInfoMsg:     // I - bold green letter
            result += greenBold + type_l.at(type) + reset;
            break;
        case QtWarningMsg:  // W - dark orange letter and text
            result += darkOrange + type_l.at(type) + reset;
            break;
        case QtCriticalMsg: // E - red bold letter and text
            result += redBold + type_l.at(type) + reset;
            break;
        case QtFatalMsg:    // F - dark red bold letter and text
            result += darkRedBold + type_l.at(type) + reset;
            break;
        default:            // Debug, System - standard color
            result += type_l.at(type);
            break;
        }
    } else {
        result += type_l.at(type);
    }

    result += QChar::fromLatin1(' ');

    // Thread - bold when colorized
    if (!m_threads.contains(lmsg.threadId())) {
        m_threads[lmsg.threadId()] = m_threadsIndex++;
    }
    if (m_threads.count() > 1) {
        auto index = m_threads.value(lmsg.threadId());
        if (index == 0) {
            auto thread = thread_f.arg(index);
            thread.fill(QChar::fromLatin1(' '));
            result += thread;
        } else {
            if (m_colorize) {
                result += bold + thread_f.arg(index) + reset;
            } else {
                result += thread_f.arg(index);
            }
        }
    }

    // Category - dark gray when colorized
    if (qstrcmp(lmsg.category(), "default") != 0) {
        auto category = category_f.arg(QString::fromUtf8(lmsg.category()));
        if (m_colorize) {
            result += darkGray + category + reset;
        } else {
            result += category;
        }
    }

    // Space for alignment
    QString category;
    if (qstrcmp(lmsg.category(), "default") != 0) {
        category = category_f.arg(QString::fromUtf8(lmsg.category()));
    }
    if (m_maxCategoryWidth > 0) {
        auto categoryLength = category.length();
        if (categoryLength > m_categoryWidth) {
            m_categoryWidth = qMin(categoryLength, m_maxCategoryWidth);
        }
        auto spaceCount = qMax(m_categoryWidth - categoryLength, 0);
        if (spaceCount > 0) {
            result += QString(spaceCount, QChar::fromLatin1(' '));
        }
    }

    // Message - with color for Info, Warning, Error and Fatal
    if (m_colorize && type == QtInfoMsg) {
        result += green + lmsg.message() + reset;
    } else if (m_colorize && type == QtWarningMsg) {
        result += orange + lmsg.message() + reset;
    } else if (m_colorize && type == QtCriticalMsg) {
        result += redBold + lmsg.message() + reset;
    } else if (m_colorize && type == QtFatalMsg) {
        result += darkRedBold + lmsg.message() + reset;
    } else {
        result += lmsg.message();
    }

    return result;
}

} // namespace QtLogger

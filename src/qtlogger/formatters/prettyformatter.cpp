// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include "prettyformatter.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
PrettyFormatter::PrettyFormatter(bool showThread, int maxCategoryWidth)
    : m_showThreadId(showThread), m_maxCategoryWidth(maxCategoryWidth)
{
}

QTLOGGER_DECL_SPEC
QString PrettyFormatter::format(const LogMessage &lmsg)
{
    static const QString msg_f { QStringLiteral("%1 %2 %3%4%5%6") };
    static const QString time_f { QStringLiteral("dd.MM.yyyy hh:mm:ss.zzz") };
    static const QStringList type_l {
        QStringLiteral(" "), QStringLiteral("W"), QStringLiteral("E"),
        QStringLiteral("F"), QStringLiteral("I"), QStringLiteral("S")
    };
    static const QString thread_f { QStringLiteral("#%1 ") };
    static const QString category_f { QStringLiteral("[%1] ") };

    QString thread;
    if (m_showThreadId) {
        if (!m_threads.contains(lmsg.threadId())) {
            m_threads[lmsg.threadId()] = m_threadsIndex++;
        }
        if (m_threads.count() > 1) {
            const auto index = m_threads.value(lmsg.threadId());
            thread = thread_f.arg(index);
            if (index == 0) {
                thread.fill(QChar::fromLatin1(' '));
            }
        }
    }

    QString category;
    if (qstrcmp(lmsg.category(), "default") != 0) {
        category = category_f.arg(QString::fromUtf8(lmsg.category()));
    }

    QString space;
    if (m_maxCategoryWidth > 0) {
        int categoryLength = category.length();
        if (categoryLength > m_categoryWidth && categoryLength <= m_maxCategoryWidth) {
            m_categoryWidth = categoryLength;
        }
        space.fill(QChar::fromLatin1(' '), qMax(m_categoryWidth - categoryLength, 0));
    }

    auto result = msg_f.arg(lmsg.time().toString(time_f), type_l.at(lmsg.type()), thread,
                            category, space, lmsg.message());

    return result;
}

} // namespace QtLogger

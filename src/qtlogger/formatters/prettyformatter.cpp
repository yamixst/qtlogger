// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "prettyformatter.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
PrettyFormatter::PrettyFormatter(bool showThread, int maxCategoryWidth)
    : m_showThread(showThread), m_maxCategoryWidth(maxCategoryWidth)
{
}

QTLOGGER_DECL_SPEC
bool PrettyFormatter::process(DebugMessage &dmesg)
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
    if (m_showThread) {
        if (!m_threads.contains(dmesg.threadId())) {
            m_threads[dmesg.threadId()] = m_threadsIndex++;
        }
        if (m_threads.count() > 1) {
            const auto index = m_threads.value(dmesg.threadId());
            thread = thread_f.arg(index);
            if (index == 0) {
                thread.fill(QChar::fromLatin1(' '));
            }
        }
    }

    QString category;
    if (qstrcmp(dmesg.category(), "default") != 0) {
        category = category_f.arg(QString::fromUtf8(dmesg.category()));
    }

    QString space;
    if (m_maxCategoryWidth > 0) {
        int categoryLength = category.length();
        if (categoryLength > m_categoryWidth && categoryLength <= m_maxCategoryWidth) {
            m_categoryWidth = categoryLength;
        }
        space.fill(QChar::fromLatin1(' '), qMax(m_categoryWidth - categoryLength, 0));
    }

    auto result = msg_f.arg(dmesg.time().toString(time_f), type_l.at(dmesg.type()), thread,
                            category, space, dmesg.message());

    dmesg.setFormattedMessage(result);

    return true;
}

} // namespace QtLogger

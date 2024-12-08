// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#ifdef QTLOGGER_JOURNAL

#include "journalsink.h"

#ifdef QTLOGGER_JOURNAL
#    include <systemd/sd-journal.h>
#endif

#include "../formatters/nullformatter.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
JournalSink::JournalSink()
{
    setPreprocessor(NullFormatter::instance());
}

QTLOGGER_DECL_SPEC
void JournalSink::send(const DebugMessage &dmesg)
{
#ifdef QTLOGGER_JOURNAL
    int priority = LOG_DEBUG;

    switch (dmesg.type()) {
    case QtDebugMsg:
        priority = LOG_DEBUG;
        break;
    case QtWarningMsg:
        priority = LOG_WARNING;
        break;
    case QtCriticalMsg:
        priority = LOG_ERR;
        break;
    case QtFatalMsg:
        priority = LOG_EMERG;
        break;
    case QtInfoMsg:
        priority = LOG_INFO;
        break;
    default:
        return;
    }

    const auto &file = QByteArrayLiteral("CODE_FILE=") + QByteArray(dmesg.file());
    const auto &line = QByteArrayLiteral("CODE_LINE=") + QByteArray::number(dmesg.line());

    sd_journal_print_with_location(priority, file.constData(), line.constData(), dmesg.function(),
                                   "%s", qPrintable(dmesg.formattedMessage()));

    sd_journal_send_with_location(file.constData(), line.constData(), dmesg.function(), "%s",
                                  qPrintable(dmesg.formattedMessage()), "PRIORITY=%i", priority,
                                  "CATEGORY=%s", dmesg.category(), NULL);
#else
    Q_UNUSED(dmesg);
#endif
}

} // namespace QtLogger

#endif // QTLOGGER_JOURNAL

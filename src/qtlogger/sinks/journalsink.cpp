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
void JournalSink::send(const LogMessage &logMsg)
{
#ifdef QTLOGGER_JOURNAL
    int priority = LOG_DEBUG;

    switch (logMsg.type()) {
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

    const auto &file = QByteArrayLiteral("CODE_FILE=") + QByteArray(logMsg.file());
    const auto &line = QByteArrayLiteral("CODE_LINE=") + QByteArray::number(logMsg.line());

    sd_journal_print_with_location(priority, file.constData(), line.constData(), logMsg.function(),
                                   "%s", qPrintable(logMsg.formattedMessage()));

    sd_journal_send_with_location(file.constData(), line.constData(), logMsg.function(), "%s",
                                  qPrintable(logMsg.formattedMessage()), "PRIORITY=%i", priority,
                                  "CATEGORY=%s", logMsg.category(), NULL);
#else
    Q_UNUSED(logMsg);
#endif
}

} // namespace QtLogger

#endif // QTLOGGER_JOURNAL

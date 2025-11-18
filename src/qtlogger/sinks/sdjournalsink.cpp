// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#ifdef QTLOGGER_SDJOURNAL

#include "sdjournalsink.h"

#include <systemd/sd-journal.h>

namespace QtLogger {

QTLOGGER_DECL_SPEC
void SdJournalSink::send(const LogMessage &lmsg)
{
    auto priority = LOG_DEBUG;

    switch (lmsg.type()) {
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

    const auto &file = QByteArrayLiteral("CODE_FILE=") + QByteArray(lmsg.file());
    const auto &line = QByteArrayLiteral("CODE_LINE=") + QByteArray::number(lmsg.line());

    sd_journal_print_with_location(priority, file.constData(), line.constData(), lmsg.function(),
                                   "%s", qPrintable(lmsg.formattedMessage()));

    sd_journal_send_with_location(file.constData(), line.constData(), lmsg.function(), "%s",
                                  qPrintable(lmsg.formattedMessage()), "PRIORITY=%i", priority,
                                  "CATEGORY=%s", lmsg.category(), NULL);
}

} // namespace QtLogger

#endif // QTLOGGER_SDJOURNAL

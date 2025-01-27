// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#ifdef QTLOGGER_SDJOURNAL

#include <QSharedPointer>

#include "../sink.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT SdJournalSink : public Sink
{
public:
    void send(const LogMessage &lmsg) override;
};

using SdJournalSinkPtr = QSharedPointer<SdJournalSink>;

} // namespace QtLogger

#endif // QTLOGGER_SDJOURNAL

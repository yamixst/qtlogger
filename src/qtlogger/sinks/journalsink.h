// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#ifdef QTLOGGER_JOURNAL

#include <QSharedPointer>

#include "../abstractmessagesink.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT JournalSink : public AbstractMessageSink
{
public:
    JournalSink();

    void send(const DebugMessage &dmesg) override;
};

using JournalSinkPtr = QSharedPointer<JournalSink>;

} // namespace QtLogger

#endif // QTLOGGER_JOURNAL

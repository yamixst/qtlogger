// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#ifdef QTLOGGER_OSLOG

#include <QSharedPointer>

#include "../sink.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT OslogSink : public Sink
{
public:
    void send(const LogMessage &logMsg);
};

using OslogSinkPtr = QSharedPointer<OslogSink>;

} // namespace QtLogger

#endif // QTLOGGER_OSLOG

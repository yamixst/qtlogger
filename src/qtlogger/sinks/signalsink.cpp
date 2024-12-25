// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "signalsink.h"

#include "../formatters/defaultformatter.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
SignalSink::SignalSink(QObject *parent) : QObject(parent) { }

QTLOGGER_DECL_SPEC
void SignalSink::send(const LogMessage &logMsg)
{
    Q_EMIT message(logMsg);
}

} // namespace QtLogger

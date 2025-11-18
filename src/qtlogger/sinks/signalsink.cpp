// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include "signalsink.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
SignalSink::SignalSink(QObject *parent) : QObject(parent) { }

QTLOGGER_DECL_SPEC
void SignalSink::send(const LogMessage &lmsg)
{
    Q_EMIT message(lmsg);
}

} // namespace QtLogger

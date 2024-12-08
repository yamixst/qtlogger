// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "signalsink.h"

#include "../formatters/defaultformatter.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
SignalSink::SignalSink(QObject *parent) : QObject(parent)
{
    setPreprocessor(DefaultFormatter::instance());
}

QTLOGGER_DECL_SPEC
void SignalSink::send(const DebugMessage &dmesg)
{
    Q_EMIT message(dmesg);
}

} // namespace QtLogger

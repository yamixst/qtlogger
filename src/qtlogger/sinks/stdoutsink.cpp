// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "stdoutsink.h"

#include <iostream>

#include "../formatters/defaultformatter.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
StdOutSink::StdOutSink()
{
    setPreprocessor(DefaultFormatter::instance());
}

QTLOGGER_DECL_SPEC
void StdOutSink::send(const DebugMessage &dmesg)
{
    std::cout << qPrintable(dmesg.formattedMessage()) << std::endl;
}

} // namespace QtLogger

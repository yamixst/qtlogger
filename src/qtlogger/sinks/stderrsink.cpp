// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "stderrsink.h"

#include <iostream>

#include "../formatters/defaultformatter.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
StdErrSink::StdErrSink()
{
    setPreprocessor(DefaultFormatter::instance());
}

QTLOGGER_DECL_SPEC
void StdErrSink::send(const DebugMessage &dmesg)
{
    std::cerr << qPrintable(dmesg.formattedMessage()) << std::endl;
}

} // namespace QtLogger

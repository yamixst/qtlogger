// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "stderrsink.h"

#include <iostream>

namespace QtLogger {

QTLOGGER_DECL_SPEC
void StdErrSink::send(const LogMessage &lmsg)
{
    std::cerr << qPrintable(lmsg.formattedMessage()) << std::endl;
}

QTLOGGER_DECL_SPEC
bool StdErrSink::flush()
{
    std::flush(std::cerr);
    return true;
}

} // namespace QtLogger

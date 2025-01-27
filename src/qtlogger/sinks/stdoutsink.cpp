// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "stdoutsink.h"

#include <iostream>

namespace QtLogger {

QTLOGGER_DECL_SPEC
void StdOutSink::send(const LogMessage &lmsg)
{
    std::cout << qPrintable(lmsg.formattedMessage()) << std::endl;
}

bool StdOutSink::flush()
{
    std::flush(std::cout);
    return true;
}

} // namespace QtLogger

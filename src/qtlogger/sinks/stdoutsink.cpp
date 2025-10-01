// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "stdoutsink.h"

#include <iostream>

namespace QtLogger {

QTLOGGER_DECL_SPEC
StdOutSink::StdOutSink(ColorMode colorMode)
    : ColoredConsole(colorMode)
{
    updateColorsEnabled();
}

QTLOGGER_DECL_SPEC
void StdOutSink::send(const LogMessage &lmsg)
{
    if (m_colorsEnabled) {
        std::cout << qPrintable(colorize(lmsg.formattedMessage(), lmsg.type())) << std::endl;
    } else {
        std::cout << qPrintable(lmsg.formattedMessage()) << std::endl;
    }
}

QTLOGGER_DECL_SPEC
bool StdOutSink::flush()
{
    std::flush(std::cout);
    return true;
}

QTLOGGER_DECL_SPEC
bool StdOutSink::isTty() const
{
    return isStdOutTty();
}

} // namespace QtLogger
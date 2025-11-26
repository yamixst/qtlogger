// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "stderrsink.h"

#include <iostream>

namespace QtLogger {

QTLOGGER_DECL_SPEC
StdErrSink::StdErrSink(ColorMode colorMode)
    : ColoredConsole(colorMode)
{
    updateColorsEnabled();
}

QTLOGGER_DECL_SPEC
void StdErrSink::send(const LogMessage &lmsg)
{
    if (m_colorsEnabled) {
        std::cerr << qPrintable(colorize(lmsg.formattedMessage(), lmsg.type())) << std::endl;
    } else {
        std::cerr << qPrintable(lmsg.formattedMessage()) << std::endl;
    }
}

QTLOGGER_DECL_SPEC
bool StdErrSink::flush()
{
    std::flush(std::cerr);
    return true;
}

QTLOGGER_DECL_SPEC
bool StdErrSink::isTty() const
{
    return isStdErrTty();
}

} // namespace QtLogger
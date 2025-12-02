// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QSharedPointer>

#include "../logger_global.h"
#include "../sink.h"
#include "coloredconsole.h"

namespace QtLogger {

class QTLOGGER_EXPORT StdErrSink : public Sink, public ColoredConsole
{
public:
    explicit StdErrSink(ColorMode colorMode = ColorMode::Never);

    void send(const LogMessage &lmsg) override;
    bool flush() override;

protected:
    bool isTty() const override;
};

using StdErrSinkPtr = QSharedPointer<StdErrSink>;

} // namespace QtLogger

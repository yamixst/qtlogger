// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#ifdef QTLOGGER_ANDROIDLOG

#include <QSharedPointer>

#include "../abstractmessagesink.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT AndroidLogSink : public AbstractMessageSink
{
public:
    AndroidLogSink();

    void send(const DebugMessage &dmesg) override;
};

using AndroidLogSinkPtr = QSharedPointer<AndroidLogSink>;

} // namespace QtLogger

#endif // QTLOGGER_ANDROIDLOG

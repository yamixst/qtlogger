// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QObject>

#include "../abstractmessagesink.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT SignalSink : public QObject, AbstractMessageSink
{
    Q_OBJECT

public:
    explicit SignalSink(QObject *parent = nullptr);

    void send(const DebugMessage &dmesg) override;

Q_SIGNALS:
    void message(const QtLogger::DebugMessage &dmesg);
};

using SignalSinkPtr = QSharedPointer<SignalSink>;

} // namespace QtLogger

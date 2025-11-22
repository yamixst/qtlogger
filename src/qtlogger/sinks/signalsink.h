// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>

#include "../logger_global.h"
#include "../sink.h"

namespace QtLogger {

class QTLOGGER_EXPORT SignalSink : public QObject, public Sink
{
    Q_OBJECT

public:
    explicit SignalSink(QObject *parent = nullptr);

    void send(const LogMessage &lmsg) override;

Q_SIGNALS:
    void message(const QtLogger::LogMessage &lmsg);
};

using SignalSinkPtr = QSharedPointer<SignalSink>;

} // namespace QtLogger

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QObject>

#include "../sink.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT SignalSink : public QObject, Sink
{
    Q_OBJECT

public:
    explicit SignalSink(QObject *parent = nullptr);

    void send(const LogMessage &logMsg) override;

Q_SIGNALS:
    void message(const QtLogger::LogMessage &logMsg);
};

using SignalSinkPtr = QSharedPointer<SignalSink>;

} // namespace QtLogger

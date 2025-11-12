// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QScopedPointer>
#include <QSharedPointer>

#include "../logger_global.h"
#include "filesink.h"

namespace QtLogger {

class QTLOGGER_EXPORT RotatingFileSink : public FileSink
{
public:
    constexpr static int DefaultMaxFileSize = 1 * 1024 * 1024; // 1 MB
    constexpr static int DefaultMaxFileCount = 5;

    enum Option
    {
        None = 0x00,
        RotationOnStartup = 0x01,
        RotationDaily = 0x02,
        Compression = 0x04
    };

    Q_DECLARE_FLAGS(Options, Option)

    explicit RotatingFileSink(const QString &path,
                              int maxFileSize = DefaultMaxFileSize,
                              int maxFileCount = DefaultMaxFileCount,
                              Options options = Option::None);
    ~RotatingFileSink() override;

    void send(const LogMessage &lmsg) override;

private:
    class RotatingFileSinkPrivate;
    QScopedPointer<RotatingFileSinkPrivate> d;
    Q_DISABLE_COPY(RotatingFileSink)
};

using RotatingFileSinkPtr = QSharedPointer<RotatingFileSink>;

} // namespace QtLogger

Q_DECLARE_OPERATORS_FOR_FLAGS(QtLogger::RotatingFileSink::Options)
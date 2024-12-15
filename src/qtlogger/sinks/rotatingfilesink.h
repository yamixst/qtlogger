// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "filesink.h"
#include "../logger_global.h"

namespace QtLogger {

constexpr int RotatingFileDefaultMaxFileSize = 1024 * 1024;
constexpr int RotatingFileDefaultMaxFileCount = 3;
constexpr int RotatingFileCountLimit = 1024;

class QTLOGGER_EXPORT RotatingFileSink : public FileSink
{
public:
    explicit RotatingFileSink(const QString &path, int maxFileSize = RotatingFileDefaultMaxFileSize,
                              int maxFileCount = RotatingFileDefaultMaxFileCount);

    void send(const LogMessage &logMsg) override;

private:
    void rotate();

private:
    int m_maxFileSize = RotatingFileDefaultMaxFileSize;
    int m_maxFileCount = RotatingFileDefaultMaxFileCount;
};

using RotatingFileSinkPtr = QSharedPointer<RotatingFileSink>;

} // namespace QtLogger

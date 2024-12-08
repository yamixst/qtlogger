// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "iodevicesink.h"
#include "../logger_global.h"

class QFile;

namespace QtLogger {

class QTLOGGER_EXPORT FileSink : public IODeviceSink
{
public:
    explicit FileSink(const QString &path);
    ~FileSink() override;

    bool flush() override;

protected:
    QFile *file() const;
};

using FileSinkPtr = QSharedPointer<FileSink>;

} // namespace QtLogger

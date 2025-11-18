// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QSharedPointer>

#include "../logger_global.h"
#include "iodevicesink.h"

QT_FORWARD_DECLARE_CLASS(QFile)

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

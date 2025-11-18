// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QSharedPointer>
#include <QScopedPointer>

#include "../formatter.h"
#include "../logger_global.h"

namespace QtLogger {

using PatternFormatterPtr = QSharedPointer<class PatternFormatter>;

class QTLOGGER_EXPORT PatternFormatter : public Formatter
{
public:
    explicit PatternFormatter(const QString &pattern);
    ~PatternFormatter() override;

    QString format(const LogMessage &lmsg) override;

private:
    class PatternFormatterPrivate;
    QScopedPointer<PatternFormatterPrivate> d;
    Q_DISABLE_COPY(PatternFormatter)
};

} // namespace QtLogger

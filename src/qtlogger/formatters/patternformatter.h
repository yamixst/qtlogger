// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "../abstractmessageformatter.h"
#include "../logger_global.h"

namespace QtLogger {

using PatternFormatterPtr = QSharedPointer<class PatternFormatter>;

class QTLOGGER_EXPORT PatternFormatter : public AbstractMessageFormatter
{
public:
    explicit PatternFormatter(const QString &pattern);

    QString format(const LogMessage &logMsg) const override;

private:
    QString m_pattern;
};

} // namespace QtLogger

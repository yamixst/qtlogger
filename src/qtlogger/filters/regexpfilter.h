// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QRegularExpression>
#include <QSharedPointer>

#include "../filter.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT RegExpFilter : public Filter
{
public:
    explicit RegExpFilter(const QRegularExpression &regExp);
    explicit RegExpFilter(const QString &regExp);

    bool filter(const LogMessage &logMsg) const override;

private:
    QRegularExpression m_regExp;
};

using RegExpFilterPtr = QSharedPointer<RegExpFilter>;

} // namespace QtLogger

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QRegularExpression>
#include <QSharedPointer>

#include "../abstractmessagefilter.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT RegExpFilter : public AbstractMessageFilter
{
public:
    explicit RegExpFilter(const QRegularExpression &regExp);
    explicit RegExpFilter(const QString &regExp);

    bool filter(const DebugMessage &dmesg) const override;

private:
    QRegularExpression m_regExp;
};

using RegExpFilterPtr = QSharedPointer<RegExpFilter>;

} // namespace QtLogger

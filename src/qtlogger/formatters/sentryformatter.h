// Copyright (C) 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QSharedPointer>

#include "../formatter.h"
#include "../logger_global.h"

namespace QtLogger {

using SentryFormatterPtr = QSharedPointer<class SentryFormatter>;

class QTLOGGER_EXPORT SentryFormatter : public Formatter
{
public:
    explicit SentryFormatter(const QString &sdkName = QStringLiteral("qtlogger.sentry"),
                             const QString &sdkVersion = QStringLiteral("1.0.0"));

    static SentryFormatterPtr instance()
    {
        static const auto s_instance = SentryFormatterPtr::create();
        return s_instance;
    }

    QString format(const LogMessage &lmsg) override;

private:
    QString m_sdkName;
    QString m_sdkVersion;
};

} // namespace QtLogger
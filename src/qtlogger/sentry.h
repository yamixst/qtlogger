// Copyright (C) 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QUrl>

#include "logger_global.h"

namespace QtLogger {

inline QString sentryUrl(const QString &sentryDsn)
{
    QUrl dsn(sentryDsn);
    auto publicKey = dsn.userName();
    auto host = dsn.host();
    auto projectId = dsn.path().mid(1); // Remove leading '/'

    return QString("https://%1/api/%2/store/?sentry_version=7&sentry_key=%3")
            .arg(host, projectId, publicKey);
}

inline QString sentryUrl(const QString &sentryHost,
                         const QString &sentryProjectId,
                         const QString &sentryPublicKey)
{
    return QString("https://%1/api/%2/store/?sentry_version=7&sentry_key=%3")
            .arg(sentryHost, sentryProjectId, sentryPublicKey);
}

inline QString sentryUrl()
{
    auto dsn = QString::fromLocal8Bit(qgetenv("SENTRY_DSN"));
    if (!dsn.isEmpty()) {
        return sentryUrl(dsn);
    }

    return sentryUrl(QString::fromLocal8Bit(qgetenv("SENTRY_HOST")),
                     QString::fromLocal8Bit(qgetenv("SENTRY_PROJECT_ID")),
                     QString::fromLocal8Bit(qgetenv("SENTRY_PUBLIC_KEY")));
}

inline bool checkSentryEnv()
{
    if (!qgetenv("SENTRY_DSN").isEmpty()) {
        return true;
    }

    return !qgetenv("SENTRY_HOST").isEmpty()
           && !qgetenv("SENTRY_PROJECT_ID").isEmpty()
           && !qgetenv("SENTRY_PUBLIC_KEY").isEmpty();
}

inline QList<QPair<QByteArray, QByteArray>> sentryHeaders()
{
    return {
        { "Content-Type", "application/json; charset=utf-8" }
    };
}

} // namespace QtLogger
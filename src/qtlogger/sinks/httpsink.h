// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#ifdef QTLOGGER_NETWORK

#include <QNetworkRequest>
#include <QSharedPointer>
#include <QPointer>
#include <QUrl>

#include "../sink.h"
#include "../logger_global.h"

QT_FORWARD_DECLARE_CLASS(QNetworkAccessManager)

namespace QtLogger {

class QTLOGGER_EXPORT HttpSink : public Sink
{
public:
    using Headers = QList<QPair<QByteArray, QByteArray>>;

    explicit HttpSink(const QUrl &url);
    HttpSink(const QUrl &url, const Headers &headers);
    ~HttpSink();

    void send(const LogMessage &lmsg) override;

    void setNetworkAccessManager(QNetworkAccessManager *manager);
    void setRequest(const QNetworkRequest &request);
    void setHeaders(const Headers &headers);

private:
    void init();

    QUrl m_url;
    Headers m_headers;
    QPointer<QNetworkAccessManager> m_manager;
    QNetworkRequest m_request;
};

using HttpSinkPtr = QSharedPointer<HttpSink>;

} // namespace QtLogger

#endif // QTLOGGER_NETWORK


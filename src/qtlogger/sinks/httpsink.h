// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

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
    explicit HttpSink(const QUrl &url);
    ~HttpSink();

    void send(const LogMessage &lmsg) override;

    void setNetworkAccessManager(QNetworkAccessManager *manager);
    void setRequest(const QNetworkRequest &request);

private:
    QUrl m_url;
    QPointer<QNetworkAccessManager> m_manager;
    QNetworkRequest m_request;
};

using HttpSinkPtr = QSharedPointer<HttpSink>;

} // namespace QtLogger

#endif // QTLOGGER_NETWORK


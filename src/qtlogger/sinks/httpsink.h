// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QNetworkRequest>
#include <QSharedPointer>
#include <QPointer>
#include <QUrl>

#include "../abstractmessagesink.h"
#include "../logger_global.h"

QT_FORWARD_DECLARE_CLASS(QNetworkAccessManager)

namespace QtLogger {

class QTLOGGER_EXPORT HttpSink : public AbstractMessageSink
{
public:
    enum Format {
        None,
        Raw,
        Default,
        Json
    };

    explicit HttpSink(const QUrl &url, Format format = Default);
    ~HttpSink();

    void send(const DebugMessage &dmesg) override;

    void setNetworkAccessManager(QNetworkAccessManager *manager);
    void setRequest(const QNetworkRequest &request);

private:
    QUrl m_url;
    QPointer<QNetworkAccessManager> m_manager;
    QNetworkRequest m_request;
};

using HttpSinkPtr = QSharedPointer<HttpSink>;

} // namespace QtLogger

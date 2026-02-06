// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#ifdef QTLOGGER_NETWORK

#include "httpsink.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "../logger.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
HttpSink::HttpSink(const QUrl &url) : m_url(url)
{
    init();
}

QTLOGGER_DECL_SPEC
HttpSink::HttpSink(const QUrl &url, const Headers &headers) : m_url(url), m_headers(headers)
{
    init();
}

QTLOGGER_DECL_SPEC
void HttpSink::init()
{
    m_manager = new QNetworkAccessManager();

#ifndef QTLOGGER_NO_THREAD
    if (m_manager->thread() != Logger::instance()->ownThread()) {
        m_manager->moveToThread(Logger::instance()->ownThread());
    }
#endif

    m_request.setUrl(m_url);
    for (const auto &header : m_headers) {
        m_request.setRawHeader(header.first, header.second);
    }
}

QTLOGGER_DECL_SPEC
HttpSink::~HttpSink()
{
    if (!m_manager.isNull()) {
        delete m_manager.data();
    }
}

QTLOGGER_DECL_SPEC
void HttpSink::send(const LogMessage &lmsg)
{
    if (!Logger::instance()->ownThreadIsRunning()) {
        if (!m_manager.isNull() && !m_manager->property("activeReply").isValid())
            m_manager->deleteLater();
        m_manager = new QNetworkAccessManager();
    }

    if (lmsg.hasAttribute("mime_type")) {
        m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                            QStringLiteral("%1; charset=utf-8")
                                    .arg(lmsg.attribute("mime_type").toByteArray()));
    } else {
        m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                            QStringLiteral("text/plain; charset=utf-8"));
    }

    auto reply = m_manager->post(m_request, lmsg.formattedMessage().toUtf8());

    QObject::connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);

    if (!Logger::instance()->ownThreadIsRunning()) {
        m_manager->setProperty("activeReply", QVariant::fromValue(reply));
        QObject::connect(reply, &QNetworkReply::finished, m_manager, &QObject::deleteLater);
    }
}

QTLOGGER_DECL_SPEC
void HttpSink::setNetworkAccessManager(QNetworkAccessManager *manager)
{
    if (!m_manager.isNull()) {
        delete m_manager.data();
    }
    m_manager = manager;
}

QTLOGGER_DECL_SPEC
void HttpSink::setRequest(const QNetworkRequest &request)
{
    m_request = request;
}

QTLOGGER_DECL_SPEC
void HttpSink::setHeaders(const Headers &headers)
{
    m_headers = headers;
    for (const auto &header : m_headers) {
        m_request.setRawHeader(header.first, header.second);
    }
}

} // namespace QtLogger

#endif // QTLOGGER_NETWORK

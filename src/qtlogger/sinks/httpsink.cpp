// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

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
    m_manager = new QNetworkAccessManager;

#ifndef QTLOGGER_NO_THREAD
    if (m_manager->thread() != Logger::instance()->ownThread()) {
        m_manager->moveToThread(Logger::instance()->ownThread());
    }
#endif

    m_request.setUrl(m_url);
}

QTLOGGER_DECL_SPEC
HttpSink::~HttpSink()
{
    if (!m_manager.isNull()) {
        delete m_manager.data();
    }
}

QTLOGGER_DECL_SPEC
void HttpSink::send(const LogMessage &logMsg)
{
    if (!Logger::instance()->ownThreadIsRunning()) {
        if (!m_manager.isNull() && !m_manager->property("activeReply").isValid())
            m_manager->deleteLater();
        m_manager = new QNetworkAccessManager;
    }

    if (logMsg.hasAttribute("mime_type")) {
        m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                            QStringLiteral("%1; charset=utf-8")
                                    .arg(logMsg.attribute("mime_type").toByteArray()));
    } else {
        m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                            QStringLiteral("text/plain; charset=utf-8"));
    }

    auto reply = m_manager->post(m_request, logMsg.formattedMessage().toUtf8());

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

} // namespace QtLogger

#endif // QTLOGGER_NETWORK

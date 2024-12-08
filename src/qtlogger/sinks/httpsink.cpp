// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#ifdef QTLOGGER_NETWORK

#include "httpsink.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "../formatters/defaultformatter.h"
#include "../formatters/jsonformatter.h"
#include "../formatters/nullformatter.h"

#include "../logger.h"

#include <iostream>

namespace QtLogger {

QTLOGGER_DECL_SPEC
HttpSink::HttpSink(const QUrl &url, Format format) : m_url(url)
{
    m_manager = new QNetworkAccessManager;

#ifndef QTLOGGER_NO_THREAD
    if (m_manager->thread() != Logger::instance()->thread()) {
        m_manager->moveToThread(Logger::instance()->thread());
    }
#endif
    m_manager->setParent(Logger::instance());

    switch (format) {
    case None:
        setPreprocessor(AbstractMessageProcessorPtr());
        break;
    case Raw:
        setPreprocessor(NullFormatter::instance());
        break;
    case Default:
        setPreprocessor(DefaultFormatter::instance());
        break;
    case Json:
        setPreprocessor(JsonFormatter::instance());
        break;
    }

    m_request.setUrl(m_url);

    if (format == Json) {
        m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                            QStringLiteral("application/json; charset=utf-8"));
    } else {
        m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                            QStringLiteral("text/plain; charset=utf-8"));
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
void HttpSink::send(const DebugMessage &dmesg)
{
    if (!Logger::instance()->ownThreadIsRunning()) {
        if (!m_manager.isNull() && !m_manager->property("activeReply").isValid())
            m_manager->deleteLater();
        m_manager = new QNetworkAccessManager;
    }

    auto reply = m_manager->post(m_request, dmesg.formattedMessage().toUtf8());

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

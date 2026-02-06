// Copyright (C) 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include "sentryformatter.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>
#include <QUuid>

namespace QtLogger {

namespace {

QString qtMsgTypeToSentryLevel(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("debug");
    case QtInfoMsg:
        return QStringLiteral("info");
    case QtWarningMsg:
        return QStringLiteral("warning");
    case QtCriticalMsg:
        return QStringLiteral("error");
    case QtFatalMsg:
        return QStringLiteral("fatal");
    default:
        return QStringLiteral("info");
    }
}

} // namespace

SentryFormatter::SentryFormatter(const QString &sdkName, const QString &sdkVersion)
    : m_sdkName(sdkName), m_sdkVersion(sdkVersion)
{
}

QTLOGGER_DECL_SPEC
QString SentryFormatter::format(const LogMessage &lmsg)
{
    QJsonObject event;

    // Required: Event ID (UUID without dashes)
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    auto eventId = QUuid::createUuid().toString(QUuid::Id128);
#else
    auto eventId = QUuid::createUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}')).remove(QLatin1Char('-'));
#endif
    event[QStringLiteral("event_id")] = eventId;

    // Required: Timestamp in ISO 8601 format
    event[QStringLiteral("timestamp")] = lmsg.time().toUTC().toString(Qt::ISODate);

    // Platform
    event[QStringLiteral("platform")] = QStringLiteral("native");

    // Severity level
    event[QStringLiteral("level")] = qtMsgTypeToSentryLevel(lmsg.type());

    // Logger name (category)
    auto category = QString::fromLatin1(lmsg.category());
    if (!category.isEmpty() && category != QLatin1String("default")) {
        event[QStringLiteral("logger")] = category;
    }

    // Message
    QJsonObject message;
    message[QStringLiteral("formatted")] = lmsg.message();
    event[QStringLiteral("message")] = message;

    // Transaction/culprit (function name)
    if (lmsg.function() && strlen(lmsg.function()) > 0) {
        event[QStringLiteral("culprit")] = QString::fromLatin1(lmsg.function());
    }

    // Tags
    QJsonObject tags;
    tags[QStringLiteral("qt_version")] = QString::fromLatin1(qVersion());
    if (lmsg.hasAttribute(QStringLiteral("app_name"))) {
        tags[QStringLiteral("app_name")] = lmsg.attribute(QStringLiteral("app_name")).toString();
    }
    if (lmsg.hasAttribute(QStringLiteral("app_version"))) {
        tags[QStringLiteral("app_version")] = lmsg.attribute(QStringLiteral("app_version")).toString();
    }
    event[QStringLiteral("tags")] = tags;

    // Extra context
    QJsonObject extra;
    extra[QStringLiteral("line")] = lmsg.line();
    if (lmsg.file() && strlen(lmsg.file()) > 0) {
        extra[QStringLiteral("file")] = QString::fromLatin1(lmsg.file());
    }
    extra[QStringLiteral("thread_id")] = QString::number(lmsg.threadId());

    // Add custom attributes to extra
    const auto attrs = lmsg.attributes();
    for (auto it = attrs.cbegin(); it != attrs.cend(); ++it) {
        // Skip already handled attributes
        if (it.key() == QLatin1String("app_name") || it.key() == QLatin1String("app_version")
            || it.key() == QLatin1String("mime_type")) {
            continue;
        }
        extra[it.key()] = QJsonValue::fromVariant(it.value());
    }
    event[QStringLiteral("extra")] = extra;

    // Contexts
    QJsonObject contexts;

    // OS context
    QJsonObject osContext;
    osContext[QStringLiteral("name")] = QSysInfo::productType();
    osContext[QStringLiteral("version")] = QSysInfo::productVersion();
    osContext[QStringLiteral("kernel_version")] = QSysInfo::kernelVersion();
    osContext[QStringLiteral("build")] = QSysInfo::buildAbi();
    contexts[QStringLiteral("os")] = osContext;

    // Device context
    QJsonObject deviceContext;
    deviceContext[QStringLiteral("arch")] = QSysInfo::currentCpuArchitecture();
    if (lmsg.hasAttribute(QStringLiteral("host_name"))) {
        deviceContext[QStringLiteral("name")] = lmsg.attribute(QStringLiteral("host_name")).toString();
    }
    contexts[QStringLiteral("device")] = deviceContext;

    // Runtime context
    QJsonObject runtimeContext;
    runtimeContext[QStringLiteral("name")] = QStringLiteral("Qt");
    runtimeContext[QStringLiteral("version")] = QString::fromLatin1(qVersion());
    contexts[QStringLiteral("runtime")] = runtimeContext;

    event[QStringLiteral("contexts")] = contexts;

    // SDK info
    QJsonObject sdk;
    sdk[QStringLiteral("name")] = m_sdkName;
    sdk[QStringLiteral("version")] = m_sdkVersion;
    event[QStringLiteral("sdk")] = sdk;

    // Fingerprint (for grouping similar events)
    QJsonArray fingerprint;
    fingerprint.append(qtMsgTypeToSentryLevel(lmsg.type()));
    fingerprint.append(category.isEmpty() ? QStringLiteral("default") : category);
    fingerprint.append(lmsg.message().left(100)); // First 100 chars of message
    event[QStringLiteral("fingerprint")] = fingerprint;

    return QString::fromUtf8(QJsonDocument(event).toJson(QJsonDocument::Compact));
}

} // namespace QtLogger
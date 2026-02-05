// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QTimer>
#include <QSysInfo>

#include <qtlogger/qtlogger.h>

// Sentry DSN format: https://<public_key>@<host>/<project_id>
// Example: https://abc123@o123456.ingest.sentry.io/1234567

// Replace with your Sentry DSN components
const QString SENTRY_PUBLIC_KEY = "your_public_key_here";
const QString SENTRY_HOST = "o123456.ingest.sentry.io";
const QString SENTRY_PROJECT_ID = "1234567";

// Build Sentry Store API endpoint URL
QString sentryStoreUrl()
{
    return QString("https://%1/api/%2/store/?sentry_version=7&sentry_key=%3")
            .arg(SENTRY_HOST)
            .arg(SENTRY_PROJECT_ID)
            .arg(SENTRY_PUBLIC_KEY);
}

// Map Qt message types to Sentry severity levels
QString qtMsgTypeToSentryLevel(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:   return "debug";
    case QtInfoMsg:    return "info";
    case QtWarningMsg: return "warning";
    case QtCriticalMsg: return "error";
    case QtFatalMsg:   return "fatal";
    default:           return "info";
    }
}

// Custom Sentry formatter function
QString formatForSentry(const QtLogger::LogMessage &lmsg)
{
    QJsonObject event;

    // Required: Event ID (UUID without dashes)
    QString eventId = QUuid::createUuid().toString(QUuid::Id128);
    event["event_id"] = eventId;

    // Required: Timestamp in ISO 8601 format
    event["timestamp"] = lmsg.time().toUTC().toString(Qt::ISODate);

    // Platform
    event["platform"] = "native";

    // Severity level
    event["level"] = qtMsgTypeToSentryLevel(lmsg.type());

    // Logger name (category)
    QString category = QString::fromLatin1(lmsg.category());
    if (!category.isEmpty() && category != "default") {
        event["logger"] = category;
    }

    // Message
    QJsonObject message;
    message["formatted"] = lmsg.message();
    event["message"] = message;

    // Transaction/culprit (function name)
    if (lmsg.function() && strlen(lmsg.function()) > 0) {
        event["culprit"] = QString::fromLatin1(lmsg.function());
    }

    // Tags
    QJsonObject tags;
    tags["qt_version"] = QString(qVersion());
    if (lmsg.hasAttribute("app_name")) {
        tags["app_name"] = lmsg.attribute("app_name").toString();
    }
    if (lmsg.hasAttribute("app_version")) {
        tags["app_version"] = lmsg.attribute("app_version").toString();
    }
    event["tags"] = tags;

    // Extra context
    QJsonObject extra;
    extra["line"] = lmsg.line();
    if (lmsg.file() && strlen(lmsg.file()) > 0) {
        extra["file"] = QString::fromLatin1(lmsg.file());
    }
    extra["thread_id"] = QString::number(lmsg.threadId());

    // Add custom attributes to extra
    const auto attrs = lmsg.attributes();
    for (auto it = attrs.cbegin(); it != attrs.cend(); ++it) {
        // Skip already handled attributes
        if (it.key() == "app_name" || it.key() == "app_version" ||
            it.key() == "mime_type") {
            continue;
        }
        extra[it.key()] = QJsonValue::fromVariant(it.value());
    }
    event["extra"] = extra;

    // Contexts
    QJsonObject contexts;

    // OS context
    QJsonObject osContext;
    osContext["name"] = QSysInfo::productType();
    osContext["version"] = QSysInfo::productVersion();
    osContext["kernel_version"] = QSysInfo::kernelVersion();
    osContext["build"] = QSysInfo::buildAbi();
    contexts["os"] = osContext;

    // Device context
    QJsonObject deviceContext;
    deviceContext["arch"] = QSysInfo::currentCpuArchitecture();
    if (lmsg.hasAttribute("host_name")) {
        deviceContext["name"] = lmsg.attribute("host_name").toString();
    }
    contexts["device"] = deviceContext;

    // Runtime context
    QJsonObject runtimeContext;
    runtimeContext["name"] = "Qt";
    runtimeContext["version"] = QString(qVersion());
    contexts["runtime"] = runtimeContext;

    event["contexts"] = contexts;

    // SDK info
    QJsonObject sdk;
    sdk["name"] = "qtlogger.sentry";
    sdk["version"] = "1.0.0";
    event["sdk"] = sdk;

    // Fingerprint (for grouping similar events)
    QJsonArray fingerprint;
    fingerprint.append(qtMsgTypeToSentryLevel(lmsg.type()));
    fingerprint.append(category.isEmpty() ? "default" : category);
    fingerprint.append(lmsg.message().left(100)); // First 100 chars of message
    event["fingerprint"] = fingerprint;

    return QString::fromUtf8(QJsonDocument(event).toJson(QJsonDocument::Compact));
}

// Custom attribute handler to set MIME type for Sentry
QVariantHash sentryAttrsHandler(const QtLogger::LogMessage &)
{
    return {
        { "mime_type", "application/json" }
    };
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("SentryExample");
    app.setApplicationVersion("1.0.0");

    // Configure QtLogger with Sentry integration
    gQtLogger
        .moveToOwnThread()  // Async logging for non-blocking HTTP requests
        .addAppInfo()       // Add app name, version, etc.
        .addHostInfo()      // Add hostname

        // Pipeline 1: Console output for local debugging
        .pipeline()
            .formatPretty(true)
            .sendToStdErr()
        .end()

        // Pipeline 2: Send warnings and errors to Sentry
        .pipeline()
            .filterLevel(QtWarningMsg)  // Only warnings, critical, and fatal
            .filterDuplicate()          // Prevent spam to Sentry
            .attrHandler(sentryAttrsHandler)
            .format(formatForSentry)
            .sendToHttp(sentryStoreUrl())
        .end();

    gQtLogger.installMessageHandler();

    // Test logging
    qDebug() << "This is a debug message (not sent to Sentry)";
    qInfo() << "This is an info message (not sent to Sentry)";
    qWarning() << "This is a warning message (sent to Sentry)";
    qCritical() << "This is a critical error (sent to Sentry)";

    // Example with category
    QLoggingCategory lc("network");
    qCWarning(lc) << "Connection timeout after 30 seconds";

    // Give time for async HTTP requests to complete
    QTimer::singleShot(2000, &app, [&app]() {
        qInfo() << "Shutting down...";
        app.quit();
    });

    return app.exec();
}
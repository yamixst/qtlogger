// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>

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

// HTTP headers for Sentry API
QList<QPair<QByteArray, QByteArray>> sentryHeaders()
{
    return {
        { "Content-Type", "application/json; charset=utf-8" }
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
        .addSysInfo()       // Add OS, kernel, CPU architecture info
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
            .formatToSentry()
            .sendToHttp(sentryStoreUrl(), sentryHeaders())
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
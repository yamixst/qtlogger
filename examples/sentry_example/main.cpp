// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>

#include <qtlogger/qtlogger.h>

// Sentry DSN format: https://<public_key>@<host>/<project_id>
// Example: https://abc123@o123456.ingest.sentry.io/1234567
//
// Environment variables (use either SENTRY_DSN or all three individual variables):
//   SENTRY_DSN        - Full Sentry DSN URL (alternative to individual variables)
//   SENTRY_PUBLIC_KEY - Sentry public key
//   SENTRY_HOST       - Sentry host (e.g., o123456.ingest.sentry.io)
//   SENTRY_PROJECT_ID - Sentry project ID

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("SentryExample");
    app.setApplicationVersion("1.0.0");

    // Check environment variables - either SENTRY_DSN or individual variables
    auto sentryDsn = qEnvironmentVariable("SENTRY_DSN");
    QString sentryUrl;

    if (!sentryDsn.isEmpty()) {
        // Parse DSN: https://<public_key>@<host>/<project_id>
        QUrl dsn(sentryDsn);
        auto publicKey = dsn.userName();
        auto host = dsn.host();
        auto projectId = dsn.path().mid(1); // Remove leading '/'

        sentryUrl = QString("https://%1/api/%2/store/?sentry_version=7&sentry_key=%3")
                .arg(host, projectId, publicKey);
    } else {
        auto sentryHost = qEnvironmentVariable("SENTRY_HOST");
        auto sentryProjectId = qEnvironmentVariable("SENTRY_PROJECT_ID");
        auto sentryPublicKey = qEnvironmentVariable("SENTRY_PUBLIC_KEY");

        if (sentryHost.isEmpty() || sentryProjectId.isEmpty() || sentryPublicKey.isEmpty()) {
            qWarning() << "Missing required environment variables.";
            qWarning() << "Set SENTRY_DSN or all of: SENTRY_HOST, SENTRY_PROJECT_ID, SENTRY_PUBLIC_KEY";
            if (sentryHost.isEmpty())
                qWarning() << "  SENTRY_HOST is not set";
            if (sentryProjectId.isEmpty())
                qWarning() << "  SENTRY_PROJECT_ID is not set";
            if (sentryPublicKey.isEmpty())
                qWarning() << "  SENTRY_PUBLIC_KEY is not set";
            return 1;
        }

        sentryUrl = QString("https://%1/api/%2/store/?sentry_version=7&sentry_key=%3")
                .arg(sentryHost, sentryProjectId, sentryPublicKey);
    }

    QList<QPair<QByteArray, QByteArray>> sentryHeaders = {
        { "Content-Type", "application/json; charset=utf-8" }
    };

    // Configure QtLogger with Sentry integration
    gQtLogger
        .moveToOwnThread()  // Async logging for non-blocking HTTP requests

        // Pipeline 1: Console output for local debugging
        .pipeline()
            .formatPretty(true)
            .sendToStdErr()
        .end()

        // Pipeline 2: Send warnings and errors to Sentry
        .pipeline()
            .addAppInfo()               // Add app name, version, etc.
            .addSysInfo()               // Add OS, kernel, CPU architecture info
            .addHostInfo()              // Add hostname
            .filterLevel(QtWarningMsg)  // Only warnings, critical, and fatal
            .filterDuplicate()          // Prevent spam to Sentry
            .formatToSentry()
            .sendToHttp(sentryUrl, sentryHeaders)
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

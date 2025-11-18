// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QString>
#include <QThread>
#include <QTimer>

#include <qtlogger/qtlogger.h>

Q_LOGGING_CATEGORY(lcApp, "app")
Q_LOGGING_CATEGORY(lcDatabase, "database")
Q_LOGGING_CATEGORY(lcNetwork, "network")
Q_LOGGING_CATEGORY(lcSecurity, "security")

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const auto logFileName = QStringLiteral("rotating_app.log");
    const int maxFileSize = 1 * 1024 * 1024; // 1 MB
    const int rotationCount = 10;

    gQtLogger.addSeqNumber()
            .format("%{seq_number:0>6} %{time} %{type:^8} [%{category:>10!}] %{message}")
            .sendToFile(logFileName, maxFileSize, rotationCount,
                        QtLogger::RotatingFileSink::RotationOnStartup
                                | QtLogger::RotatingFileSink::RotationDaily
                                | QtLogger::RotatingFileSink::Compression);

    gQtLogger.installMessageHandler();

    QTimer::singleShot(0, &app, [&app, logFileName]() {
        qCInfo(lcApp) << "Log rotation demo started.";

        for (int i = 0; i < 50000; ++i) {
            qCInfo(lcApp) << "Application log message number";
            qCWarning(lcDatabase) << "Database warning message number";
            qCDebug(lcNetwork) << "Network debug message number";
            qCCritical(lcSecurity) << "Security critical message number";
        }

        qCInfo(lcApp) << "Log rotation demo completed. Check log files in" << QDir::currentPath();

        app.quit();
    });

    return app.exec();
}

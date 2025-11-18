// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>

// #include "../../qtlogger.h"

#include <qtlogger/qtlogger.h>

Q_LOGGING_CATEGORY(lc, "MyCategory")

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Logging without QtLogger

    qDebug() << "Hello world";
    qInfo() << "Hello world";
    qWarning() << "Hello world";
    qCritical() << "Hello world";

    qCDebug(lc) << "Hello world";
    qCInfo(lc) << "Hello world";
    qCWarning(lc) << "Hello world";
    qCCritical(lc) << "Hello world";

    // Logging with QtLogger

    gQtLogger.configure(app.applicationDirPath() + "/config.ini");

    qDebug() << "Hello QtLogger";
    qInfo() << "Hello QtLogger";
    qWarning() << "Hello QtLogger";
    qCritical() << "Hello QtLogger";

    qCDebug(lc) << "Hello QtLogger";
    qCInfo(lc) << "Hello QtLogger";
    qCWarning(lc) << "Hello QtLogger";
    qCCritical(lc) << "Hello QtLogger";

    QTimer::singleShot(0, &app, [&app] () { app.quit(); });

    return app.exec();
}

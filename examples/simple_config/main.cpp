// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>
#include <QString>

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

    gQtLogger
        .addAppInfo()
        .addAppUuid()
        .pipeline()
            .addSeqNumber()
            .filter("^(?!.*password|.*secret).*$")
            .filterLevel(QtWarningMsg)
            .addSeqNumber("seq_number_after_filter")
            .format([](const QtLogger::LogMessage &lmsg){
                return QString("[%1|%2] %3 %4")
                        .arg(lmsg.attribute("seq_number").toInt())
                        .arg(lmsg.attribute("seq_number_after_filter").toInt())
                        .arg(lmsg.message())
                        .arg(lmsg.category() == QLatin1String("default") ? QString() : lmsg.category());
            })
            .sendToStdErr()
        .end()
        .pipeline()
            .formatToJson()
            .sendToFile("log.json", 100 * 1024, 3)
        .end();

    gQtLogger.installMessageHandler();

    qDebug() << "Hello QtLogger";
    qInfo() << "Hello QtLogger";
    qWarning() << "Hello QtLogger";
    qCritical() << "Hello QtLogger";

    qCDebug(lc) << "Hello QtLogger";
    qCInfo(lc) << "Hello QtLogger";
    qCWarning(lc) << "Hello QtLogger";
    qCCritical(lc) << "Hello QtLogger";

    QTimer::singleShot(0, &app, [&app]() { app.quit(); });

    return app.exec();
}

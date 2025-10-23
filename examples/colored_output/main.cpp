// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>

#include <qtlogger/qtlogger.h>

Q_LOGGING_CATEGORY(lc, "MyCategory")

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Example: Colored console output
    // Colors are applied based on log level:
    // - Debug:    Gray
    // - Info:     Green
    // - Warning:  Yellow
    // - Critical: Red
    // - Fatal:    Bold bright red

    gQtLogger
        .pipeline()
            .formatPretty()
            .sendToStdOut(true)  // Enable colored output
        .end();

    gQtLogger.installMessageHandler();

    qDebug() << "This is a debug message (gray)";
    qInfo() << "This is an info message (green)";
    qWarning() << "This is a warning message (yellow)";
    qCritical() << "This is a critical message (red)";

    qCDebug(lc) << "Debug with category";
    qCInfo(lc) << "Info with category";
    qCWarning(lc) << "Warning with category";
    qCCritical(lc) << "Critical with category";

    // You can also use StdOutSink directly with ColorMode options:
    //
    // ColorMode::Auto   - Auto-detect if output is a TTY (default)
    // ColorMode::Always - Always use colors
    // ColorMode::Never  - Never use colors
    //
    // Example:
    //   auto sink = StdOutSinkPtr::create(ColorMode::Always);
    //   pipeline->append(sink);

    QTimer::singleShot(0, &app, [&app]() { app.quit(); });

    return app.exec();
}
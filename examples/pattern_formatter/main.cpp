// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>

#include <qtlogger/qtlogger.h>

Q_LOGGING_CATEGORY(lcNetwork, "network")
Q_LOGGING_CATEGORY(lcDatabase, "database")

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // PatternFormatter supports the following placeholders:
    //
    // %{message}      - Log message text
    // %{type}         - Log level (debug, info, warning, critical)
    // %{category}     - Logging category name
    // %{threadid}     - Thread ID
    // %{file}         - Source file name
    // %{shortfile [basedir]} - Source file name without base directory
    // %{line}         - Line number
    // %{function}     - Function name
    // %{func}         - Short function name
    // %{time}         - Timestamp in ISO format (default)
    // %{time FORMAT}  - Custom time format (e.g., "yyyy-MM-dd HH:mm:ss.zzz")
    // %{time process} - Seconds since process start
    // %{time boot}    - Seconds since system boot
    // %{ATTR}         - Custom attribute value (e.g., %{seq_number})
    // %{if-debug}...%{endif}    - Show content only for debug messages
    // %{if-info}...%{endif}     - Show content only for info messages
    // %{if-warning}...%{endif}  - Show content only for warnings
    // %{if-critical}...%{endif} - Show content only for critical messages
    // %%              - Escaped percent sign

    gQtLogger
        .addSeqNumber()
        .format(QStringLiteral(
            "#%{seq_number} "
            "%{time process}s "
            "%{time yyyy-MM-dd HH:mm:ss.zzz} "
            "%{shortfile}:%{line} - %{func}: "
            "/%{threadid}/ "
            "[%{category}] "
            "%{if-debug}DBG%{endif}"
            "%{if-info}INF%{endif}"
            "%{if-warning}WRN%{endif}"
            "%{if-critical}CRT%{endif}"
            ": %{message}"))
        .sendToStdOut();

    gQtLogger.installMessageHandler();

    qDebug() << "This is a debug message";
    qInfo() << "This is an info message";
    qWarning() << "This is a warning message";
    qCritical() << "This is a critical message";

    qCDebug(lcNetwork) << "Network debug";
    qCInfo(lcNetwork) << "Network info";
    qCWarning(lcDatabase) << "Database warning";
    qCCritical(lcDatabase) << "Database critical";

    QTimer::singleShot(0, &app, [&app]() { app.quit(); });

    return app.exec();
}

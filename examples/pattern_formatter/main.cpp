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
    // Basic placeholders:
    // %{message}      - Log message text
    // %{type}         - Log level (debug, info, warning, critical)
    // %{category}     - Logging category name
    // %{threadid}     - Thread ID as string
    // %{qthreadptr}   - Pointer to the current QThread object
    // %{file}         - Full source file path
    // %{shortfile}    - Source file name without base directory
    // %{shortfile BASEDIR} - Source file name with custom base directory to strip
    // %{line}         - Line number in source file
    // %{function}     - Full function signature with cleanup
    // %{func}         - Short function name without arguments
    //
    // Time placeholders:
    // %{time}         - Timestamp in ISO 8601 format (yyyy-MM-ddTHH:mm:ss.zzz)
    // %{time FORMAT}  - Custom time format using Qt date/time format specifiers
    //                   Examples: "yyyy-MM-dd HH:mm:ss.zzz", "HH:mm:ss", "dd.MM.yyyy"
    // %{time process} - Seconds since process start (floating point)
    // %{time boot}    - Seconds since system boot (floating point)
    //
    // Custom attributes:
    // %{ATTR}         - Custom attribute value (e.g., %{seq_number}, %{user_id})
    // %{ATTR?}        - Optional attribute (no output if not set)
    // %{ATTR?N}       - Optional attribute, remove N characters before if not set
    // %{ATTR?N,M}     - Optional attribute, remove N chars before and M chars after if not set
    //
    // Fixed-width formatting:
    // Any placeholder can have a format spec appended after colon:
    //   %{PLACEHOLDER:[fill][align][width][!]}
    //
    // Alignment characters:
    //   <             - Left align (default for strings)
    //   >             - Right align
    //   ^             - Center align
    //
    // Fill character:
    //   Any single character before alignment (default is space)
    //
    // Width:
    //   Positive integer specifying minimum field width
    //
    // Truncation (!):
    //   Add ! at the end to truncate values longer than width
    //   Without ! values longer than width are not truncated
    //
    // Examples:
    //   %{type:<10}     - Left align, width 10, pad with spaces
    //   %{type:>10}     - Right align, width 10, pad with spaces
    //   %{type:^10}     - Center align, width 10 (extra padding goes right)
    //   %{type:*<10}    - Left align, width 10, pad with '*'
    //   %{type:_^15}    - Center align, width 15, pad with '_'
    //   %{type:<8!}     - Left align, width 8, truncate if longer
    //   %{type:*^10!}   - Center align, width 10, pad with '*', truncate if longer
    //   %{category:<6!} - Left align, width 6, truncate if longer
    //   %{time process:>9} - Right align process time to width 9
    //
    // Conditional blocks:
    // %{if-debug}...%{endif}    - Show content only for debug messages
    // %{if-info}...%{endif}     - Show content only for info messages
    // %{if-warning}...%{endif}  - Show content only for warnings
    // %{if-critical}...%{endif} - Show content only for critical messages
    //
    // Special characters:
    // %%              - Escaped percent sign (literal %)

    gQtLogger
        .addSeqNumber()
        .format("#%{seq_number?:0>4} "
                "::%{myattr?2,1:^20} "
                "%{time process:>9}s "
                "%{time yyyy-MM-dd HH:mm:ss.zzz} "
                "%{shortfile}:%{line} - %{func}: "
                "%{qthreadptr} "
                "[%{category:<6!}] "
                "%{if-debug}DBG%{endif}"
                "%{if-info}INF%{endif}"
                "%{if-warning}WRN%{endif}"
                "%{if-critical}CRT%{endif}"
                ": %{message}")
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

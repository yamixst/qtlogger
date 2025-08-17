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
    // Fixed-width formatting (Python-style):
    // Any placeholder can have a format spec appended after colon:
    //   %{PLACEHOLDER:[fill][align][width]}     - padding only
    //   %{PLACEHOLDER:[width!]}                 - truncation only
    //   %{PLACEHOLDER:[align][width!]}          - truncation with direction
    //   %{PLACEHOLDER:[fill][align][width!]}    - truncation AND padding
    //
    // Alignment characters:
    //   <             - Left align / truncate from right (keep first N chars)
    //   >             - Right align / truncate from left (keep last N chars)
    //   ^             - Center align (extra padding goes right)
    //
    // Fill character:
    //   Any single character before alignment (default is space)
    //
    // Width:
    //   Positive integer specifying field width
    //
    // Truncation (! suffix):
    //   Without fill char: truncate only, NO padding for shorter values
    //   With fill char: truncate AND pad (both apply)
    //
    // Examples (padding only - no !):
    //   %{type:<10}       - Left align, width 10, pad with spaces
    //   %{type:>10}       - Right align, width 10, pad with spaces
    //   %{type:^10}       - Center align, width 10 (extra padding goes right)
    //   %{type:*<10}      - Left align, width 10, pad with '*'
    //   %{type:_^15}      - Center align, width 15, pad with '_'
    //
    // Examples (truncation only - ! without fill char):
    //   %{type:10!}       - Max 10 chars, truncate from right, no padding
    //   %{type:<10!}      - Max 10 chars, truncate from right (keep first 10)
    //   %{type:>10!}      - Max 10 chars, truncate from left (keep last 10)
    //
    // Examples (truncation AND padding - ! with fill char):
    //   %{type:*<10!}     - Truncate/pad to exactly 10, fill with '*', left align
    //   %{type: >10!}     - Truncate/pad to exactly 10, fill with space, right align
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

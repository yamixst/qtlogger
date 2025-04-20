// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QtCore/QtGlobal>
#include <QFlags>
#include <QSettings>
#include <QString>

#include "logger_global.h"

namespace QtLogger {

class Logger;
class Pipeline;

enum class SinkType {
    Unknown = 0x00,
    StdOut = 0x01,
    StdErr = 0x02,
    Syslog = 0x04,
    SdJournal = 0x8,
    PlatformStdLog = 0x10,
    File = 0x40,
    RotatingFile = 0x80
};

Q_DECLARE_FLAGS(SinkTypeFlags, SinkType)

/** Configure logger with specified sink types and parameters
 *
 * @param pipeline Target pipeline to configure
 * @param types Sink types to enable
 * @param path File path for file-based sinks
 * @param maxFileSize Maximum file size for rotating files (0 = no rotation)
 * @param maxFileCount Maximum number of rotating files
 * @param async Enable async processing (requires threading support)
 */
QTLOGGER_EXPORT void configurePipeline(Pipeline *pipeline, const SinkTypeFlags &types,
                                       const QString &path = {}, int maxFileSize = 0,
                                       int maxFileCount = 0, bool async = false);

/** Configure logger with specified sink types and parameters (int version)
 *
 * @param pipeline Target pipeline to configure
 * @param types Sink types to enable (as int flags)
 * @param path File path for file-based sinks
 * @param maxFileSize Maximum file size for rotating files (0 = no rotation)
 * @param maxFileCount Maximum number of rotating files
 * @param async Enable async processing (requires threading support)
 */
QTLOGGER_EXPORT void configurePipeline(Pipeline *pipeline, int types, const QString &path = {},
                                       int maxFileSize = 0, int maxFileCount = 0,
                                       bool async = false);

/** Configure logger from QSettings
 *
 * Supported settings:
 * logger/filter_rules = [<category>|*][.debug|.info|.warning|.critical]=true|false;...
 * logger/regexp_filter = <regexp>
 * logger/message_pattern = <string>
 * logger/stdout = true|false
 * logger/stderr = true|false
 * logger/platform_std_log = true|false
 * logger/syslog_ident = <string>
 * logger/sdjournal = true|false
 * logger/path = <string>
 * logger/max_file_size = <int>
 * logger/max_file_count = <int>
 * logger/async = true|false
 * logger/http_url = <string>
 * logger/http_msg_format = <string>
 *
 * @param pipeline Target pipeline to configure
 * @param settings QSettings object to read from
 * @param group Settings group name
 */
QTLOGGER_EXPORT void configurePipeline(Pipeline *pipeline, const QSettings &settings,
                                       const QString &group = QStringLiteral("logger"));

/** Configure logger from ini file
 *
 * @param pipeline Target pipeline to configure
 * @param path Path to ini file
 * @param group Settings group name
 */
QTLOGGER_EXPORT void configurePipeline(Pipeline *pipeline, const QString &path,
                                       const QString &group = QStringLiteral("logger"));

/** Configure logger instance with specified sink types and parameters
 *
 * @param logger Target logger to configure
 * @param types Sink types to enable
 * @param path File path for file-based sinks
 * @param maxFileSize Maximum file size for rotating files (0 = no rotation)
 * @param maxFileCount Maximum number of rotating files
 * @param async Enable async processing (requires threading support)
 */
QTLOGGER_EXPORT void configureLogger(Logger *logger, const SinkTypeFlags &types,
                                     const QString &path = {}, int maxFileSize = 0,
                                     int maxFileCount = 0, bool async = false);

/** Configure logger instance with specified sink types and parameters (int version)
 *
 * @param logger Target logger to configure
 * @param types Sink types to enable (as int flags)
 * @param path File path for file-based sinks
 * @param maxFileSize Maximum file size for rotating files (0 = no rotation)
 * @param maxFileCount Maximum number of rotating files
 * @param async Enable async processing (requires threading support)
 */
QTLOGGER_EXPORT void configureLogger(Logger *logger, int types, const QString &path = {},
                                     int maxFileSize = 0, int maxFileCount = 0, bool async = false);

/** Configure logger instance from QSettings
 *
 * @param logger Target logger to configure
 * @param settings QSettings object to read from
 * @param group Settings group name
 */
QTLOGGER_EXPORT void configureLogger(Logger *logger, const QSettings &settings,
                                     const QString &group = QStringLiteral("logger"));

/** Configure logger instance from ini file
 *
 * @param logger Target logger to configure
 * @param path Path to ini file
 * @param group Settings group name
 */
QTLOGGER_EXPORT void configureLogger(Logger *logger, const QString &path,
                                     const QString &group = QStringLiteral("logger"));

} // namespace QtLogger

Q_DECLARE_OPERATORS_FOR_FLAGS(QtLogger::SinkTypeFlags)
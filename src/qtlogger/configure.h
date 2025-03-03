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

QTLOGGER_EXPORT void configure(Pipeline *pipeline, const SinkTypeFlags &types,
                                       const QString &path = {}, int maxFileSize = 0,
                                       int maxFileCount = 0, bool async = false);

QTLOGGER_EXPORT void configure(Pipeline *pipeline, int types, const QString &path = {},
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
QTLOGGER_EXPORT void configure(Pipeline *pipeline, const QSettings &settings,
                                       const QString &group = QStringLiteral("logger"));

QTLOGGER_EXPORT void configure(Pipeline *pipeline, const QString &path,
                                       const QString &group = QStringLiteral("logger"));
} // namespace QtLogger

Q_DECLARE_OPERATORS_FOR_FLAGS(QtLogger::SinkTypeFlags)

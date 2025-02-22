// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "configure.h"

#include <QtCore/QtGlobal>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QUrl>

#include "logger.h"
#include "pipeline.h"
#include "sortedpipeline.h"
#include "filters/regexpfilter.h"
#include "formatters/patternformatter.h"
#include "formatters/prettyformatter.h"
#include "sinks/filesink.h"
#include "sinks/platformstdsink.h"
#include "sinks/rotatingfilesink.h"
#include "sinks/stderrsink.h"
#include "sinks/stdoutsink.h"

#ifdef QTLOGGER_NETWORK
#    include "sinks/httpsink.h"
#endif

#ifdef QTLOGGER_SYSLOG
#    include "sinks/syslogsink.h"
#endif

#ifdef QTLOGGER_SDJOURNAL
#    include "sinks/sdjournalsink.h"
#endif

#ifdef QTLOGGER_OSLOG
#    include "sinks/oslogsink.h"
#endif

#ifdef QTLOGGER_ANDROIDLOG
#    include "sinks/androidlogsink.h"
#endif

#ifndef QTLOGGER_NO_THREAD
#    include "ownthreadhandler.h"
#endif

#include <iostream>

namespace QtLogger {

QTLOGGER_DECL_SPEC
void configurePipeline(Pipeline *pipeline, const SinkTypeFlags &types, const QString &path,
                       int maxFileSize, int maxFileCount, bool async)
{
    if (!pipeline) {
        return;
    }

    *pipeline << PrettyFormatter::instance();

    if (types.testFlag(SinkType::StdOut)) {
        *pipeline << StdOutSinkPtr::create();
    }

    if (types.testFlag(SinkType::StdErr)) {
        *pipeline << StdErrSinkPtr::create();
    }

    if (types.testFlag(SinkType::PlatformStdLog)) {
        *pipeline << PlatformStdSinkPtr::create();
    }

#ifdef QTLOGGER_SYSLOG
    if (types.testFlag(SinkType::Syslog)) {
        *pipeline << SyslogSinkPtr::create(QFileInfo(path).baseName());
    }
#endif

#ifdef QTLOGGER_SDJOURNAL
    if (types.testFlag(SinkType::SdJournal)) {
        *pipeline << SdJournalSinkPtr::create();
    }
#endif

    if (!path.isEmpty()) {
        if (maxFileSize == 0) {
            *pipeline << FileSinkPtr::create(path);
        } else {
            *pipeline << RotatingFileSinkPtr::create(path, maxFileSize, maxFileCount);
        }
    }

    Q_UNUSED(async)
}

QTLOGGER_DECL_SPEC
void configurePipeline(Pipeline *pipeline, int types, const QString &path, int maxFileSize,
                       int maxFileCount, bool async)
{
    configurePipeline(pipeline, SinkTypeFlags(QFlag(types)), path, maxFileSize, maxFileCount, async);
}

QTLOGGER_DECL_SPEC
void configurePipeline(Pipeline *pipeline, const QSettings &settings, const QString &group)
{
    if (!pipeline) {
        return;
    }

    *pipeline << PrettyFormatter::instance();

    const auto filterRules = settings.value(group + QStringLiteral("/filter_rules")).toString();
    if (!filterRules.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "configurePipeline: filterRules: " << filterRules.toStdString() << std::endl;
#endif
        QLoggingCategory::setFilterRules(
                QString(filterRules).replace(QChar::fromLatin1(';'), QChar::fromLatin1('\n')));
    }

    const auto regExpFilter = settings.value(group + QStringLiteral("/regexp_filter")).toString();
    if (!regExpFilter.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "configurePipeline: filter: " << regExpFilter.toStdString() << std::endl;
#endif
        *pipeline << RegExpFilterPtr::create(regExpFilter);
    }

    const auto messagePattern =
            settings.value(group + QStringLiteral("/message_pattern")).toString();
    if (!messagePattern.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "configurePipeline: messagePattern: " << messagePattern.toStdString()
                  << std::endl;
#endif
        *pipeline << PatternFormatterPtr::create(messagePattern);
    }

    if (settings.value(group + QStringLiteral("/stdout"), false).toBool()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "configurePipeline: stdout" << std::endl;
#endif
        *pipeline << StdOutSinkPtr::create();
    }

    if (settings.value(group + QStringLiteral("/stderr"), false).toBool()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "configurePipeline: stderr" << std::endl;
#endif
        *pipeline << StdErrSinkPtr::create();
    }

    if (settings.value(group + QStringLiteral("/platform_std_log"), true).toBool()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "configurePipeline: platform_std_log" << std::endl;
#endif
        *pipeline << PlatformStdSinkPtr::create();
    }

#ifdef QTLOGGER_SYSLOG
    const auto syslogIdent = settings.value(group + QStringLiteral("/syslog_ident")).toString();
    if (!syslogIdent.isEmpty()) {
#    ifdef QTLOGGER_DEBUG
        std::cerr << "configurePipeline: syslogIdent: " << syslogIdent.toStdString() << std::endl;
#    endif
        *pipeline << SyslogSinkPtr::create(syslogIdent);
    }
#endif

#ifdef QTLOGGER_SDJOURNAL
    if (settings.value(group + QStringLiteral("/sdjournal"), false).toBool()) {
#    ifdef QTLOGGER_DEBUG
        std::cerr << "configurePipeline: sd-journal" << std::endl;
#    endif
        *pipeline << SdJournalSinkPtr::create();
    }
#endif

    const auto path = settings.value(group + QStringLiteral("/path")).toString();
    if (!path.isEmpty()) {
        const auto maxFileSize = settings.value(group + QStringLiteral("/max_file_size"),
                                                RotatingFileDefaultMaxFileSize)
                                         .toInt();

        const auto maxFileCount = settings.value(group + QStringLiteral("/max_file_count"),
                                                 RotatingFileDefaultMaxFileCount)
                                          .toInt();

#ifdef QTLOGGER_DEBUG
        std::cerr << "configurePipeline: path: " << path.toStdString()
                  << " maxFileSize: " << maxFileSize << " maxFileCount: " << maxFileCount
                  << std::endl;
#endif

        *pipeline << RotatingFileSinkPtr::create(path, maxFileSize, maxFileCount);
    }

#ifdef QTLOGGER_NETWORK
    const auto httpUrl = settings.value(group + QStringLiteral("/http_url")).toString();
    if (!httpUrl.isEmpty()) {
        const auto httpMsgFormat = settings.value(group + QStringLiteral("/http_msg_format"),
                                                  QStringLiteral("default"))
                                           .toString();
        // TODO: add support for http_msg_format (json)
        *pipeline << HttpSinkPtr::create(QUrl(httpUrl));
    }
#endif
}

QTLOGGER_DECL_SPEC
void configurePipeline(Pipeline *pipeline, const QString &path, const QString &group)
{
    configurePipeline(pipeline, QSettings(path, QSettings::IniFormat), group);
}

QTLOGGER_DECL_SPEC
void configureLogger(Logger *logger, const SinkTypeFlags &types, const QString &path,
                     int maxFileSize, int maxFileCount, bool async)
{
    if (!logger) {
        return;
    }

    configurePipeline(logger, types, path, maxFileSize, maxFileCount, async);

#ifndef QTLOGGER_NO_THREAD
    if (async) {
        auto *ownThreadLogger = dynamic_cast<OwnThreadHandler<SimplePipeline>*>(logger);
        if (ownThreadLogger) {
            ownThreadLogger->moveToOwnThread();
        }
    }
#else
    Q_UNUSED(async)
#endif

    logger->installMessageHandler();
}

QTLOGGER_DECL_SPEC
void configureLogger(Logger *logger, int types, const QString &path, int maxFileSize,
                     int maxFileCount, bool async)
{
    configureLogger(logger, SinkTypeFlags(QFlag(types)), path, maxFileSize, maxFileCount, async);
}

QTLOGGER_DECL_SPEC
void configureLogger(Logger *logger, const QSettings &settings, const QString &group)
{
    if (!logger) {
        return;
    }

    configurePipeline(logger, settings, group);

#ifndef QTLOGGER_NO_THREAD
    if (settings.value(group + QStringLiteral("/async"), false).toBool()) {
#    ifdef QTLOGGER_DEBUG
        std::cerr << "configureLogger: async" << std::endl;
#    endif
        auto *ownThreadLogger = dynamic_cast<OwnThreadHandler<SimplePipeline>*>(logger);
        if (ownThreadLogger) {
            ownThreadLogger->moveToOwnThread();
        }
    }
#endif

    logger->installMessageHandler();
}

QTLOGGER_DECL_SPEC
void configureLogger(Logger *logger, const QString &path, const QString &group)
{
    configureLogger(logger, QSettings(path, QSettings::IniFormat), group);
}

} // namespace QtLogger
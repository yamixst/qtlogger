// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "configure.h"

#include <QFileInfo>
#include <QLoggingCategory>
#include <QUrl>
#include <QtCore/QtGlobal>

#include "filters/regexpfilter.h"
#include "formatters/patternformatter.h"
#include "formatters/prettyformatter.h"
#include "pipeline.h"
#include "simplepipeline.h"
#include "sinks/filesink.h"
#include "sinks/platformstdsink.h"
#include "sinks/rotatingfilesink.h"
#include "sinks/stderrsink.h"
#include "sinks/stdoutsink.h"
#include "utils.h"

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

namespace QtLogger {

QTLOGGER_DECL_SPEC
void configure(Pipeline *pipeline, const SinkTypeFlags &types, const QString &path,
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
            *pipeline << RotatingFileSinkPtr::create(path, maxFileSize, maxFileCount, 
                RotatingFileSink::RotationOnStartup);
        }
    }

#ifndef QTLOGGER_NO_THREAD
    if (async) {
        auto *ownThreadLogger = dynamic_cast<OwnThreadHandler<SimplePipeline> *>(pipeline);
        if (ownThreadLogger) {
            ownThreadLogger->moveToOwnThread();
        }
    }
#else
    Q_UNUSED(async)
#endif
}

QTLOGGER_DECL_SPEC
void configure(Pipeline *pipeline, int types, const QString &path, int maxFileSize,
                       int maxFileCount, bool async)
{
    configure(pipeline, SinkTypeFlags(QFlag(types)), path, maxFileSize, maxFileCount,
                      async);
}

QTLOGGER_DECL_SPEC
void configure(Pipeline *pipeline, const QSettings &settings, const QString &group)
{
    if (!pipeline) {
        return;
    }

    *pipeline << PrettyFormatter::instance();

    const auto filterRules = settings.value(group + QStringLiteral("/filter_rules")).toString();
    if (!filterRules.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "configure: filterRules: " << filterRules.toStdString() << std::endl;
#endif
        QtLogger::setFilterRules(filterRules);
    }

    const auto regExpFilter = settings.value(group + QStringLiteral("/regexp_filter")).toString();
    if (!regExpFilter.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "configure: filter: " << regExpFilter.toStdString() << std::endl;
#endif
        *pipeline << RegExpFilterPtr::create(regExpFilter);
    }

    const auto messagePattern =
            settings.value(group + QStringLiteral("/message_pattern")).toString();
    if (!messagePattern.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "configure: messagePattern: " << messagePattern.toStdString()
                  << std::endl;
#endif
        *pipeline << PatternFormatterPtr::create(messagePattern);
    }

    if (settings.value(group + QStringLiteral("/stdout"), false).toBool()) {
        const bool stdoutColor = settings.value(group + QStringLiteral("/stdout_color"), false).toBool();
#ifdef QTLOGGER_DEBUG
        std::cerr << "configure: stdout (color=" << stdoutColor << ")" << std::endl;
#endif
        *pipeline << StdOutSinkPtr::create(stdoutColor ? ColorMode::Always : ColorMode::Never);
    }

    if (settings.value(group + QStringLiteral("/stderr"), false).toBool()) {
        const bool stderrColor = settings.value(group + QStringLiteral("/stderr_color"), false).toBool();
#ifdef QTLOGGER_DEBUG
        std::cerr << "configure: stderr (color=" << stderrColor << ")" << std::endl;
#endif
        *pipeline << StdErrSinkPtr::create(stderrColor ? ColorMode::Always : ColorMode::Never);
    }

    if (settings.value(group + QStringLiteral("/platform_std_log"), true).toBool()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "configure: platform_std_log" << std::endl;
#endif
        *pipeline << PlatformStdSinkPtr::create();
    }

#ifdef QTLOGGER_SYSLOG
    const auto syslogIdent = settings.value(group + QStringLiteral("/syslog_ident")).toString();
    if (!syslogIdent.isEmpty()) {
#    ifdef QTLOGGER_DEBUG
        std::cerr << "configure: syslogIdent: " << syslogIdent.toStdString() << std::endl;
#    endif
        *pipeline << SyslogSinkPtr::create(syslogIdent);
    }
#endif

#ifdef QTLOGGER_SDJOURNAL
    if (settings.value(group + QStringLiteral("/sdjournal"), false).toBool()) {
#    ifdef QTLOGGER_DEBUG
        std::cerr << "configure: sd-journal" << std::endl;
#    endif
        *pipeline << SdJournalSinkPtr::create();
    }
#endif

    const auto path = settings.value(group + QStringLiteral("/path")).toString();
    if (!path.isEmpty()) {
        const auto maxFileSize = settings.value(group + QStringLiteral("/max_file_size"),
                                                RotatingFileSink::DefaultMaxFileSize)
                                         .toInt();

        const auto maxFileCount = settings.value(group + QStringLiteral("/max_file_count"),
                                                 RotatingFileSink::DefaultMaxFileCount)
                                          .toInt();

        const auto rotateOnStartup = settings.value(group + QStringLiteral("/rotate_on_startup"),
                                                    true).toBool();

        const auto rotateDaily = settings.value(group + QStringLiteral("/rotate_daily"),
                                                false).toBool();

        const auto compress = settings.value(group + QStringLiteral("/compress_old_files"),
                                             false).toBool();

#ifdef QTLOGGER_DEBUG
        std::cerr << "configure: path: " << path.toStdString()
                  << " maxFileSize: " << maxFileSize << " maxFileCount: " << maxFileCount
                  << " rotateOnStartup: " << rotateOnStartup << " rotateDaily: " << rotateDaily
                  << " compress: " << compress << std::endl;
#endif

        RotatingFileSink::Options options = RotatingFileSink::Option::None;
        if (rotateOnStartup)
            options |= RotatingFileSink::RotationOnStartup;
        if (rotateDaily)
            options |= RotatingFileSink::RotationDaily;
        if (compress)
            options |= RotatingFileSink::Option::Compression;
        
        *pipeline << RotatingFileSinkPtr::create(path, maxFileSize, maxFileCount, options);
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

#ifndef QTLOGGER_NO_THREAD
    if (settings.value(group + QStringLiteral("/async"), false).toBool()) {
#    ifdef QTLOGGER_DEBUG
        std::cerr << "configureLogger: async" << std::endl;
#    endif
        auto *ownThreadLogger = dynamic_cast<OwnThreadHandler<SimplePipeline> *>(pipeline);
        if (ownThreadLogger) {
            ownThreadLogger->moveToOwnThread();
        }
    }
#endif
}

QTLOGGER_DECL_SPEC
void configure(Pipeline *pipeline, const QString &path, const QString &group)
{
    configure(pipeline, QSettings(path, QSettings::IniFormat), group);
}

} // namespace QtLogger

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSettings>

#ifndef QTLOGGER_NO_THREAD
#    include <QEvent>
#    include <QMutex>
#    include <QThread>
#endif

#include "handler.h"
#include "logger_global.h"

#ifndef QTLOGGER_NO_THREAD
#    include "ownthreadpipeline.h"
#else
#    include "simplepipeline.h"
#endif

#define gQtLogger QtLogger::Logger::instance()

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#    define QRMUTEX QRecursiveMutex
#else
#    define QRMUTEX QMutex
#endif

namespace QtLogger {

class QTLOGGER_EXPORT Logger :
#ifndef QTLOGGER_NO_THREAD
    public OwnThreadPipeline
#else
    public SimplePipeline
#endif
{
public:
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

    static Logger *instance();

    Logger() = default;
    ~Logger() override;

    /*
        Set global filter rules

        Format:  "[<category>|*].[debug|info|warning|critical]=true|false;..."
        Example: "app.*.debug=false;app.logger.debug=true"
    */
    static void setFilterRules(const QString &rules);

    /*
       Set global message pattern

       Following placeholders are supported:
       %{appname} %{category} %{file} %{function} %{line} %{message} %{pid} %{threadid}
       %{qthreadptr} %{type} %{time process} %{time boot} %{time [format]} %{backtrace [depth=N]
       [separator="..."]}
    */
    static void setMessagePattern(const QString &pattern);

    Logger &operator<<(const HandlerPtr &handler);

    void configure(std::initializer_list<HandlerPtr> handlers, bool async = false);

    void configure(const SinkTypeFlags &types = SinkType::PlatformStdLog, const QString &path = {},
                   int maxFileSize = 0, int maxFileCount = 0, bool async = false);

    void configure(int types, const QString &path = {}, int maxFileSize = 0, int maxFileCount = 0,
                   bool async = false);

    /*
       logger/filter_rules = [<category>|*][.debug|.info|.warning|.critical]=true|false;...
       logger/message_pattern = <string>
       logger/regexp_filter = <regexp>
       logger/stdout = true|false
       logger/stderr = true|false
       logger/platform_std_log = true|false
       logger/syslog_ident = <string>
       logger/sdjournal = true|false
       logger/path = <string>
       logger/max_file_size = <int>
       logger/max_file_count = <int>
       logger/async = true|false
    */
    void configure(const QSettings &settings, const QString &group = QStringLiteral("logger"));
    void configure(const QString &path, const QString &group = QStringLiteral("logger"));

#ifndef QTLOGGER_NO_THREAD
public:
    void lock() const;
    void unlock() const;
    QRMUTEX *mutex() const;
#endif

public:
    void installMessageHandler();
    static void restorePreviousMessageHandler();

    void processMessage(QtMsgType type, const QMessageLogContext &context, const QString &message);

    static void messageHandler(QtMsgType type, const QMessageLogContext &context,
                               const QString &message);

private:
#ifndef QTLOGGER_NO_THREAD
#    if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    mutable QRecursiveMutex m_mutex;
#    else
    mutable QMutex m_mutex { QMutex::Recursive };
#    endif
#endif
};

inline Logger &operator<<(Logger *logger, const HandlerPtr &handler)
{
    return *logger << handler;
}

inline Logger &operator<<(Logger *logger, const Pipeline &pipeline)
{
    return *logger << PipelinePtr::create(pipeline);
}

} // namespace QtLogger

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QList>
#include <QObject>
#include <QPointer>
#include <QSettings>

#ifndef QTLOGGER_NO_THREAD
#    include <QEvent>
#    include <QMutex>
#    include <QThread>
#endif

#include "abstractmessagefilter.h"
#include "abstractmessageformatter.h"
#include "abstractmessageprocessor.h"
#include "abstractmessagesink.h"
#include "logger_global.h"
#include "messagehandler.h"

#define gQtLogger QtLogger::Logger::instance()

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define QRMUTEX QRecursiveMutex
#else
#define QRMUTEX QMutex
#endif

namespace QtLogger {

class QTLOGGER_EXPORT Logger : public QObject
{
    Q_OBJECT

public:
    enum SinkTypeFlag {
        None = 0x00,
        StdOut = 0x01,
        StdErr = 0x02,
        SysLog = 0x04,
        Journal = 0x8,
        StdLog = 0x10, // For Android, iOS
        NTEventLog = 0x20,
        File = 0x40,
        RotatingFile = 0x80
    };

    Q_DECLARE_FLAGS(SinkType, SinkTypeFlag)

    static Logger *instance();

    explicit Logger(QObject *parent = nullptr);
    virtual ~Logger();

    /*
        Format:  "[<category>|*].[debug|info|warning|critical]=true|false;..."
        Example: "app.*.debug=false;app.logger.debug=true"
    */
    static void setFilterRules(const QString &rules);

    /*
       Following placeholders are supported:
       %{appname} %{category} %{file} %{function} %{line} %{message} %{pid} %{threadid}
       %{qthreadptr} %{type} %{time process} %{time boot} %{time [format]} %{backtrace [depth=N]
       [separator="..."]}
    */
    static void setMessagePattern(const QString &pattern);

    void append(const AbstractMessageProcessorPtr &processor);
    void append(std::initializer_list<AbstractMessageProcessorPtr> processors);
    void remove(const AbstractMessageProcessorPtr &processor);
    void clear();

    void appendFilter(const AbstractMessageFilterPtr &filter);
    FunctionFilterPtr appendFilter(const std::function<bool(const LogMessage &)> &function);
    RegExpFilterPtr appendFilter(const QRegularExpression &regExp);
    void clearFilters();

    void setFormatter(const AbstractMessageFormatterPtr &formatter);
    FunctionFormatterPtr setFormatter(const std::function<QString(const LogMessage &)> &function);
    PatternFormatterPtr setFormatter(const QString &pattern);
    void clearFormatters();

    void appendSink(const AbstractMessageSinkPtr &sink);
    void appendHttpSink(const QString &url, int format);
    void clearSinks();

    void appendHandler(const MessageHandlerPtr &handler);
    void clearHandlers();

    Logger &operator<<(const AbstractMessageProcessorPtr &processor);

    void configure(std::initializer_list<AbstractMessageProcessorPtr> processors,
                   bool async = false);
    void configure(const SinkType &types = StdLog, const QString &path = {}, int maxFileSize = 0,
                   int maxFileCount = 0, bool async = false);
    void configure(int types, const QString &path = {}, int maxFileSize = 0, int maxFileCount = 0,
                   bool async = false);

    /*
       logger/filter_rules = [<category>|*][.debug|.info|.warning|.critical]=true|false;...
       logger/message_pattern = <string>
       logger/regexp_filter = <regexp>
       logger/stdout = true|false
       logger/stderr = true|false
       logger/stdlog = true|false
       logger/syslog_ident = <string>
       logger/journal = true|false
       logger/path = <string>
       logger/max_file_size = <int>
       logger/max_file_count = <int>
       logger/async = true|false
    */
    void configure(const QSettings &settings, const QString &group = QStringLiteral("logger"));
    void configure(const QString &path, const QString &group = QStringLiteral("logger"));

    void flush();

#ifndef QTLOGGER_NO_THREAD
public:
    void lock() const;
    void unlock() const;
    QRMUTEX *mutex() const;

    void moveToOwnThread();
    void moveToMainThread();
    bool ownThreadIsRunning() const;
    inline QThread *ownThread() { return m_thread.data(); }

protected:
    struct LogEvent : public QEvent
    {
        LogMessage logMsg;

        LogEvent(const LogMessage &logMsg);
    };

    void customEvent(QEvent *event) override;
#endif

public:
    void installMessageHandler();
    static void restorePreviousMessageHandler();

    static void messageHandler(QtMsgType type, const QMessageLogContext &context,
                               const QString &message);

    void processMessage(QtMsgType type, const QMessageLogContext &context, const QString &message);
    void processMessage(const LogMessage &logMsg);

private:
    void processMessage(LogMessage &logMsg);

private:
    MessageHandlerPtr m_handler = MessageHandlerPtr::create();

#ifndef QTLOGGER_NO_THREAD
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    mutable QRecursiveMutex m_mutex;
#else
    mutable QMutex m_mutex { QMutex::Recursive };
#endif
    QPointer<QThread> m_thread;
#endif
};

inline Logger &operator<<(Logger *logger, const AbstractMessageProcessorPtr &processor)
{
    return *logger << processor;
}

inline Logger& operator<<(Logger* logger, const MessageHandler& handler)
{
    return *logger << MessageHandlerPtr::create(handler);
}

} // namespace QtLogger

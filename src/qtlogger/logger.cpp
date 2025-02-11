// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "logger.h"

#include <QFileInfo>
#include <QLoggingCategory>
#include <QScopedPointer>

#ifndef QTLOGGER_NO_THREAD
#    include <QAtomicPointer>
#    include <QCoreApplication>
#    include <QMutexLocker>
#endif

#include <iostream>

#include "filters/regexpfilter.h"
#include "formatters/prettyformatter.h"
#include "messagepatterns.h"
#include "setmessagepattern.h"
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

namespace QtLogger {

namespace {

#ifndef QTLOGGER_NO_THREAD
QAtomicPointer<Logger> g_activeLogger;
#else
Logger *g_activeLogger = nullptr;
#endif

QtMessageHandler g_previousMessageHandler = nullptr;

}

QTLOGGER_DECL_SPEC
Logger *Logger::instance()
{
    static QScopedPointer<Logger> s_instance;

    if (!s_instance) {
        s_instance.reset(new Logger());
    }

    return s_instance.data();
}

QTLOGGER_DECL_SPEC
Logger::~Logger()
{
#ifndef QTLOGGER_NO_THREAD
    g_activeLogger.testAndSetOrdered(this, nullptr);
#else
    if (g_activeLogger == this) {
        g_activeLogger = nullptr;
    }
#endif
}

QTLOGGER_DECL_SPEC
void Logger::setFilterRules(const QString &rules)
{
    QLoggingCategory::setFilterRules(
            QString(rules).replace(QChar::fromLatin1(';'), QChar::fromLatin1('\n')));
}

QTLOGGER_DECL_SPEC
void Logger::setMessagePattern(const QString &pattern)
{
    if (pattern.toLower() == QStringLiteral("default")) {
        QtLogger::setMessagePattern(QString::fromUtf8(DefaultMessagePattern));
        return;
    }

    if (pattern.toLower() == QStringLiteral("pretty")) {
        QtLogger::setMessagePattern(QString::fromUtf8(PrettyMessagePattern));
        return;
    }

    QtLogger::setMessagePattern(pattern);
}

QTLOGGER_DECL_SPEC
Logger &Logger::operator<<(const HandlerPtr &handler)
{
    append(handler);
    return *this;
}

QTLOGGER_DECL_SPEC
void Logger::configure(std::initializer_list<HandlerPtr> handlers, bool async)
{
#ifndef QTLOGGER_NO_THREAD
    QMutexLocker locker(mutex());
#endif

    clear();

    for (const auto &handler : handlers) {
        append(handler);
    }

#ifndef QTLOGGER_NO_THREAD
    if (async) {
        moveToOwnThread();
    }
#else
    Q_UNUSED(async)
#endif

    installMessageHandler();
}

QTLOGGER_DECL_SPEC
void Logger::configure(const SinkTypeFlags &types, const QString &path, int maxFileSize,
                       int maxFileCount, bool async)
{
#ifndef QTLOGGER_NO_THREAD
    QMutexLocker locker(mutex());
#endif

    clear();

    setFormatter(PrettyFormatter::instance());

    if (types.testFlag(SinkType::StdOut)) {
        appendSink(StdOutSinkPtr::create());
    }

    if (types.testFlag(SinkType::StdErr)) {
        appendSink(StdErrSinkPtr::create());
    }

    if (types.testFlag(SinkType::PlatformStdLog)) {
        appendSink(PlatformStdSinkPtr::create());
    }

#ifdef QTLOGGER_SYSLOG
    if (types.testFlag(SinkType::Syslog)) {
        appendSink(SyslogSinkPtr::create(QFileInfo(path).baseName()));
    }
#endif

#ifdef QTLOGGER_SDJOURNAL
    if (types.testFlag(SinkType::SdJournal)) {
        appendSink(SdJournalSinkPtr::create());
    }
#endif

    if (types.testFlag(SinkType::File) && !path.isEmpty()) {
        appendSink(FileSinkPtr::create(path));
    }

    if (types.testFlag(SinkType::RotatingFile) && !path.isEmpty()) {
        appendSink(RotatingFileSinkPtr::create(path, maxFileSize, maxFileCount));
    }

#ifndef QTLOGGER_NO_THREAD
    if (async) {
        moveToOwnThread();
    }
#else
    Q_UNUSED(async)
#endif

    installMessageHandler();
}

QTLOGGER_DECL_SPEC
void Logger::configure(int types, const QString &path, int maxFileSize, int maxFileCount,
                       bool async)
{
    configure(SinkTypeFlags(QFlag(types)), path, maxFileSize, maxFileCount, async);
}

QTLOGGER_DECL_SPEC
void Logger::configure(const QSettings &settings, const QString &group)
{
#ifndef QTLOGGER_NO_THREAD
    QMutexLocker locker(mutex());
#endif

    clear();

    setFormatter(PrettyFormatter::instance());

    const auto filterRules = settings.value(group + QStringLiteral("/filter_rules")).toString();
    if (!filterRules.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: filterRules: " << filterRules.toStdString() << std::endl;
#endif
        setFilterRules(filterRules);
    }

    const auto regExpFilter = settings.value(group + QStringLiteral("/regexp_filter")).toString();
    if (!regExpFilter.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: filter: " << regExpFilter.toStdString() << std::endl;
#endif
        appendFilter(RegExpFilterPtr::create(regExpFilter));
    }

    const auto messagePattern =
            settings.value(group + QStringLiteral("/message_pattern")).toString();
    if (!messagePattern.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: messagePattern: " << messagePattern.toStdString()
                  << std::endl;
#endif
        setFormatter(PatternFormatterPtr::create(messagePattern));
    }

    if (settings.value(group + QStringLiteral("/stdout"), false).toBool()) {
        std::cerr << "Logger::configure: stdout" << std::endl;
        appendSink(StdOutSinkPtr::create());
    }

    if (settings.value(group + QStringLiteral("/stderr"), false).toBool()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: stderr" << std::endl;
#endif
        appendSink(StdErrSinkPtr::create());
    }

    if (settings.value(group + QStringLiteral("/platform_std_log"), true).toBool()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: platform_std_log" << std::endl;
#endif
        appendSink(PlatformStdSinkPtr::create());
    }

#ifdef QTLOGGER_SYSLOG
    const auto syslogIdent = settings.value(group + QStringLiteral("/syslog_ident")).toString();
    if (!syslogIdent.isEmpty()) {
#    ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: syslogIdent: " << syslogIdent.toStdString() << std::endl;
#    endif
        appendSink(SyslogSinkPtr::create(syslogIdent));
    }
#endif

#ifdef QTLOGGER_SDJOURNAL
    if (settings.value(group + QStringLiteral("/sdjournal"), false).toBool()) {
#    ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: sd-journal" << std::endl;
#    endif
        appendSink(SdJournalSinkPtr::create());
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
        std::cerr << "Logger::configure: path: " << path.toStdString()
                  << " maxFileSize: " << maxFileSize << " maxFileCount: " << maxFileCount
                  << std::endl;
#endif

        appendSink(RotatingFileSinkPtr::create(path, maxFileSize, maxFileCount));
    }

#ifdef QTLOGGER_NETWORK
    const auto httpUrl = settings.value(group + QStringLiteral("/http_url")).toString();
    if (!httpUrl.isEmpty()) {
        const auto httpMsgFormat = settings.value(group + QStringLiteral("/http_msg_format"),
                                                  QStringLiteral("default"))
                                           .toString();
        // TODO: add support for http_msg_format (json)
        appendSink(HttpSinkPtr::create(QUrl(httpUrl)));
    }
#endif

#ifndef QTLOGGER_NO_THREAD
    if (settings.value(group + QStringLiteral("/async"), false).toBool()) {
#    ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: async" << std::endl;
#    endif
        moveToOwnThread();
    }
#endif

    installMessageHandler();
}

QTLOGGER_DECL_SPEC
void Logger::configure(const QString &path, const QString &group)
{
    configure(QSettings(path, QSettings::IniFormat), group);
}

#ifndef QTLOGGER_NO_THREAD

QTLOGGER_DECL_SPEC
void Logger::lock() const
{
    mutex()->lock();
}

QTLOGGER_DECL_SPEC
void Logger::unlock() const
{
    mutex()->unlock();
}

#endif

QTLOGGER_DECL_SPEC
void Logger::processMessage(QtMsgType type, const QMessageLogContext &context,
                            const QString &message)
{
#ifndef QTLOGGER_NO_THREAD
    QMutexLocker locker(mutex());
#endif

    LogMessage lmsg(type, context, message);
    process(lmsg);
}

QTLOGGER_DECL_SPEC
void Logger::messageHandler(QtMsgType type, const QMessageLogContext &context,
                            const QString &message)
{
#ifndef QTLOGGER_NO_THREAD
    auto logger = g_activeLogger.loadAcquire();
#else
    auto logger = g_activeLogger;
#endif

    if (!logger)
        return;

    logger->processMessage(type, context, message);
}

QTLOGGER_DECL_SPEC
void Logger::installMessageHandler()
{
#ifndef QTLOGGER_NO_THREAD
    g_activeLogger.storeRelease(this);
#else
    g_activeLogger = this;
#endif

    auto prev = qInstallMessageHandler(messageHandler);

    if (prev != messageHandler) {
        g_previousMessageHandler = prev;
    }
}

QTLOGGER_DECL_SPEC
void Logger::restorePreviousMessageHandler()
{
    if (!g_previousMessageHandler)
        return;

    auto prev = qInstallMessageHandler(g_previousMessageHandler);

    if (prev != messageHandler) {
        qInstallMessageHandler(prev);
    }

    g_previousMessageHandler = nullptr;
}

} // namespace QtLogger

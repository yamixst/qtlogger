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

#include "filters/functionfilter.h"
#include "filters/regexpfilter.h"
#include "formatters/defaultformatter.h"
#include "formatters/functionformatter.h"
#include "formatters/prettyformatter.h"
#include "messagepatterns.h"
#include "setmessagepattern.h"
#include "sinks/rotatingfilesink.h"
#include "sinks/stderrsink.h"
#include "sinks/stdlogsink.h"
#include "sinks/stdoutsink.h"

#ifdef QTLOGGER_NETWORK
#    include "sinks/httpsink.h"
#endif

#ifdef QTLOGGER_SYSLOG
#    include "sinks/syslogsink.h"
#endif

#ifdef QTLOGGER_JOURNAL
#    include "sinks/journalsink.h"
#endif

#ifdef QTLOGGER_IOSLOG
#    include "sinks/ioslogsink.h"
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
        s_instance.reset(new Logger);
        s_instance->installMessageHandler();
    }

    return s_instance.data();
}

QTLOGGER_DECL_SPEC
Logger::Logger(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QtLogger::LogMessage>("QtLogger::LogMessage");
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

    flush();

#ifndef QTLOGGER_NO_THREAD
    if (m_thread) {
        m_thread->quit();
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

    if (pattern.toLower() == QStringLiteral("default")) {
        QtLogger::setMessagePattern(QString::fromUtf8(PrettyMessagePattern));
        return;
    }

    QtLogger::setMessagePattern(pattern);
}

QTLOGGER_DECL_SPEC
void Logger::append(const AbstractMessageProcessorPtr &processor)
{
    m_handler->append(processor);
}

QTLOGGER_DECL_SPEC
void Logger::append(std::initializer_list<AbstractMessageProcessorPtr> processors)
{
    m_handler->append(processors);
}

QTLOGGER_DECL_SPEC
void Logger::remove(const AbstractMessageProcessorPtr &processor)
{
    m_handler->remove(processor);
}

QTLOGGER_DECL_SPEC
void Logger::clear()
{
    m_handler->clear();
}

QTLOGGER_DECL_SPEC
void Logger::appendFilter(const AbstractMessageFilterPtr &filter)
{
    m_handler->appendFilter(filter);
}

QTLOGGER_DECL_SPEC
FunctionFilterPtr Logger::appendFilter(const FunctionFilter::Function &function)
{
    return m_handler->appendFilter(function);
}

QTLOGGER_DECL_SPEC
RegExpFilterPtr Logger::appendFilter(const QRegularExpression &regExp)
{
    return m_handler->appendFilter(regExp);
}

QTLOGGER_DECL_SPEC
void Logger::clearFilters()
{
    m_handler->clearFilters();
}

QTLOGGER_DECL_SPEC
void Logger::setFormatter(const AbstractMessageFormatterPtr &formatter)
{
    m_handler->setFormatter(formatter);
}

QTLOGGER_DECL_SPEC
FunctionFormatterPtr Logger::setFormatter(const FunctionFormatter::Function &function)
{
    return m_handler->setFormatter(function);
}

QTLOGGER_DECL_SPEC
PatternFormatterPtr Logger::setFormatter(const QString &pattern)
{
    return m_handler->setFormatter(pattern);
}

QTLOGGER_DECL_SPEC
void Logger::clearFormatters()
{
    m_handler->clearFormatters();
}

QTLOGGER_DECL_SPEC
void Logger::appendSink(const AbstractMessageSinkPtr &sink)
{
    m_handler->appendSink(sink);
}

QTLOGGER_DECL_SPEC
void Logger::appendHttpSink(const QString &url, int format)
{
#ifdef QTLOGGER_NETWORK
    appendSink(HttpSinkPtr::create(QUrl(url), static_cast<HttpSink::Format>(format)));
#endif
}

QTLOGGER_DECL_SPEC
void Logger::clearSinks()
{
    m_handler->clearSinks();
}

QTLOGGER_DECL_SPEC
void Logger::appendHandler(const PipelineHandlerPtr &handler)
{
    m_handler->appendHandler(handler);
}

QTLOGGER_DECL_SPEC
void Logger::clearHandlers()
{
    m_handler->clearHandlers();
}

QTLOGGER_DECL_SPEC
Logger &Logger::operator<<(const AbstractMessageProcessorPtr &processor)
{
    append(processor);
    return *this;
}

QTLOGGER_DECL_SPEC
void Logger::configure(std::initializer_list<AbstractMessageProcessorPtr> processors, bool async)
{
#ifndef QTLOGGER_NO_THREAD
    QMutexLocker locker(&m_mutex);
#endif

    m_handler = PipelineHandlerPtr::create(processors);

#ifndef QTLOGGER_NO_THREAD
    if (async) {
        moveToOwnThread();
    }
#else
    Q_UNUSED(async)
#endif
}

QTLOGGER_DECL_SPEC
void Logger::configure(const SinkType &types, const QString &path, int maxFileSize,
                       int maxFileCount, bool async)
{
#ifndef QTLOGGER_NO_THREAD
    QMutexLocker locker(&m_mutex);
#endif

    clear();

    DefaultFormatter::instance()->setFormatter(PrettyFormatter::instance());

    if (types.testFlag(StdOut)) {
        appendSink(StdOutSinkPtr::create());
    }

    if (types.testFlag(StdErr)) {
        appendSink(StdErrSinkPtr::create());
    }

    if (types.testFlag(StdLog)) {
        appendSink(StdLogSinkPtr::create());
    }

#ifdef QTLOGGER_SYSLOG
    if (types.testFlag(SysLog)) {
        appendSink(SysLogSinkPtr::create(QFileInfo(path).baseName()));
    }
#endif

#ifdef QTLOGGER_JOURNAL
    if (types.testFlag(Journal)) {
        appendSink(JournalSinkPtr::create());
    }
#endif

    if (types.testFlag(File) && !path.isEmpty()) {
        appendSink(FileSinkPtr::create(path));
    }

    if (types.testFlag(RotatingFile) && !path.isEmpty()) {
        appendSink(RotatingFileSinkPtr::create(path, maxFileSize, maxFileCount));
    }

#ifndef QTLOGGER_NO_THREAD
    if (async) {
        moveToOwnThread();
    }
#else
    Q_UNUSED(async)
#endif
}

QTLOGGER_DECL_SPEC
void Logger::configure(int types, const QString &path, int maxFileSize, int maxFileCount,
                       bool async)
{
    configure(SinkType(QFlag(types)), path, maxFileSize, maxFileCount, async);
}

QTLOGGER_DECL_SPEC
void Logger::configure(const QSettings &settings, const QString &group)
{
#ifndef QTLOGGER_NO_THREAD
    QMutexLocker locker(&m_mutex);
#endif

    clear();

    DefaultFormatter::instance()->setFormatter(PrettyFormatter::instance());

    const auto filterRules = settings.value(group + QStringLiteral("/filter_rules")).toString();
    if (!filterRules.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: filterRules: " << filterRules.toStdString() << std::endl;
#endif
        setFilterRules(filterRules);
    }

    const auto messagePattern =
            settings.value(group + QStringLiteral("/message_pattern")).toString();
    if (!messagePattern.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: messagePattern: " << messagePattern.toStdString()
                  << std::endl;
#endif
        DefaultFormatter::instance()->setFormatter(QtLogMessageFormatter::instance());
        setMessagePattern(messagePattern);
    }

    const auto regExpFilter = settings.value(group + QStringLiteral("/regexp_filter")).toString();
    if (!regExpFilter.isEmpty()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: filter: " << regExpFilter.toStdString() << std::endl;
#endif
        appendFilter(RegExpFilterPtr::create(regExpFilter));
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

    if (settings.value(group + QStringLiteral("/stdlog"), true).toBool()) {
#ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: stdlog" << std::endl;
#endif
        appendSink(StdLogSinkPtr::create());
    }

#ifdef QTLOGGER_SYSLOG
    const auto sysLogIdent = settings.value(group + QStringLiteral("/syslog_ident")).toString();
    if (!sysLogIdent.isEmpty()) {
#    ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: sysLogIdent: " << sysLogIdent.toStdString() << std::endl;
#    endif
        appendSink(SysLogSinkPtr::create(sysLogIdent));
    }
#endif

#ifdef QTLOGGER_JOURNAL
    if (settings.value(group + QStringLiteral("/journal"), false).toBool()) {
#    ifdef QTLOGGER_DEBUG
        std::cerr << "Logger::configure: journal" << std::endl;
#    endif
        appendSink(JournalSinkPtr::create());
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
        static QMap<QString, HttpSink::Format> formats {
            { QStringLiteral("raw"), HttpSink::Raw },
            { QStringLiteral("default"), HttpSink::Default },
            { QStringLiteral("json"), HttpSink::Json },
        };
        appendSink(HttpSinkPtr::create(QUrl(httpUrl),
                                       formats.value(httpMsgFormat, HttpSink::Default)));
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
}

QTLOGGER_DECL_SPEC
void Logger::configure(const QString &path, const QString &group)
{
    configure(QSettings(path, QSettings::IniFormat), group);
}

QTLOGGER_DECL_SPEC
void Logger::flush()
{
    m_handler->flush();
}

#ifndef QTLOGGER_NO_THREAD

QTLOGGER_DECL_SPEC
void Logger::lock() const
{
    m_mutex.lock();
}

QTLOGGER_DECL_SPEC
void Logger::unlock() const
{
    m_mutex.unlock();
}

QTLOGGER_DECL_SPEC
QRMUTEX *Logger::mutex() const
{
    return &m_mutex;
}

QTLOGGER_DECL_SPEC
void Logger::moveToOwnThread()
{
    if (!m_thread) {
        m_thread = new QThread;
        if (qApp) {
            if (qApp->thread() != m_thread->thread())
                m_thread->moveToThread(qApp->thread());
            connect(qApp, &QCoreApplication::aboutToQuit, m_thread, &QThread::quit);
        }
        connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
        m_thread->start();
    }

    moveToThread(m_thread);
}

QTLOGGER_DECL_SPEC
void Logger::moveToMainThread()
{
    moveToThread(qApp->thread());

    if (m_thread) {
        m_thread->quit();
    }
}

QTLOGGER_DECL_SPEC
bool Logger::ownThreadIsRunning() const
{
    return m_thread && m_thread->isRunning();
}

namespace {

QTLOGGER_DECL_SPEC
static QEvent::Type QtLoggerEventType()
{
    // TODO: QEvent::registerEventType()
    static const QEvent::Type type = static_cast<QEvent::Type>(QEvent::User + 1000);
    return type;
}

}

QTLOGGER_DECL_SPEC
Logger::LogEvent::LogEvent(const LogMessage &logMsg) : QEvent(QtLoggerEventType()), logMsg(logMsg) { }

QTLOGGER_DECL_SPEC
void Logger::customEvent(QEvent *event)
{
    if (event->type() == QtLoggerEventType()) {
        auto ev = dynamic_cast<LogEvent *>(event);
        if (ev) {
            m_handler->process(ev->logMsg);
        }
    }
}

#endif

QTLOGGER_DECL_SPEC
void Logger::processMessage(LogMessage &logMsg)
{
    m_handler->process(logMsg);
}

QTLOGGER_DECL_SPEC
void Logger::processMessage(const LogMessage &logMsg)
{
    LogMessage __logMsg(logMsg);
    processMessage(__logMsg);
}

QTLOGGER_DECL_SPEC
void Logger::processMessage(QtMsgType type, const QMessageLogContext &context,
                            const QString &message)
{
    LogMessage logMsg(type, context, message);

#ifndef QTLOGGER_NO_THREAD
    if (!ownThreadIsRunning()) {
        processMessage(logMsg);
    } else {
        QCoreApplication::postEvent(this, new LogEvent(logMsg));
    }
#else
    processMessage(logMsg);
#endif
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

#ifndef QTLOGGER_NO_THREAD
    QMutexLocker locker(logger->mutex());
#endif

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

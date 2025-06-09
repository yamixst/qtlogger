// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "logger.h"

#include <QFileInfo>
#include <QLoggingCategory>
#include <QScopedPointer>

#ifndef QTLOGGER_NO_THREAD
#    include <QAtomicPointer>
#    include <QMutexLocker>
#endif

#include "configure.h"

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
void Logger::configure(const SinkTypeFlags &types, const QString &path, int maxFileSize,
                       int maxFileCount, bool async)
{
    QtLogger::configure(this, types, path, maxFileSize, maxFileCount, async);

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
    QtLogger::configure(this, settings, group);

    installMessageHandler();
}

QTLOGGER_DECL_SPEC
void Logger::configure(const QString &path, const QString &group)
{
    configure(QSettings(path, QSettings::IniFormat), group);
}

QTLOGGER_DECL_SPEC
Logger &Logger::operator<<(const HandlerPtr &handler)
{
    append(handler);
    return *this;
}

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

} // namespace QtLogger

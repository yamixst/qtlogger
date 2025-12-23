// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QFlags>
#include <QSettings>

#ifndef QTLOGGER_NO_THREAD
#    include <QMutex>
#endif

#include "configure.h"
#include "handler.h"
#include "logger_global.h"
#include "simplepipeline.h"

#ifndef QTLOGGER_NO_THREAD
#    include "ownthreadhandler.h"
#endif

#define gQtLogger (*QtLogger::Logger::instance())

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#    define QRMUTEX QRecursiveMutex
#else
#    define QRMUTEX QMutex
#endif

namespace QtLogger {

class QTLOGGER_EXPORT Logger :
#ifndef QTLOGGER_NO_THREAD
    public OwnThreadHandler<SimplePipeline>
#else
    public SimplePipeline
#endif
{
public:
    static Logger *instance();

    Logger() = default;
    ~Logger() override;

    void configure(const QString &path = {}, int maxFileSize = 0, int maxFileCount = 0,
                   RotatingFileSink::Options options = RotatingFileSink::Option::None,
                   bool async = true);

    void configure(const QSettings &settings, const QString &group = QStringLiteral("logger"));
    void configureFromIniFile(const QString &path, const QString &group = QStringLiteral("logger"));

    Logger &operator<<(const HandlerPtr &handler);

public:
    void installMessageHandler();
    static void restorePreviousMessageHandler();

    void processMessage(QtMsgType type, const QMessageLogContext &context, const QString &message);

    static void messageHandler(QtMsgType type, const QMessageLogContext &context,
                               const QString &message);

#ifndef QTLOGGER_NO_THREAD
public:
    void lock() const;
    void unlock() const;
    inline QRMUTEX *mutex() const { return &m_mutex; }

private:
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

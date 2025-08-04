// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QDateTime>
#include <QVariant>
#include <qlogging.h>

#ifndef QTLOGGER_NO_THREAD
#    include <QThread>
#endif

#include "logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT LogMessage
{
public:
    LogMessage() noexcept = default;

    LogMessage(QtMsgType type, const QMessageLogContext &context, const QString &message) noexcept
        : m_type(type),
          m_context(context.file, context.line, context.function, context.category),
          m_message(message)
    {
    }

    LogMessage(const LogMessage &lmsg) noexcept
        : m_file(lmsg.m_context.file),
          m_function(lmsg.m_context.function),
          m_category(lmsg.m_context.category),
          m_type(lmsg.m_type),
          m_context(m_file.constData(), lmsg.m_context.line, m_function.constData(),
                    m_category.constData()),
          m_message(lmsg.m_message),
          m_time(lmsg.m_time),
          m_threadId(lmsg.m_threadId),
          m_formattedMessage(lmsg.m_formattedMessage),
          m_attributes(lmsg.m_attributes)
    {
    }

    inline QtMsgType type() const { return m_type; }
    inline const QMessageLogContext &context() const { return m_context; }
    inline QString message() const { return m_message; }

    // Context members

    inline int line() const { return m_context.line; }
    inline const char *file() const { return m_context.file; }
    inline const char *function() const { return m_context.function; }
    inline const char *category() const { return m_context.category; }

    // System attributes

    inline QDateTime time() const { return m_time; }
    inline qint64 threadId() const
    {
#ifndef QTLOGGER_NO_THREAD
        return m_threadId;
#else
        return 0;
#endif
    }

    // Formatted message

    inline QString formattedMessage() const
    {
        return isFormatted() ? m_formattedMessage : m_message;
    }
    inline void setFormattedMessage(const QString &formattedMessage)
    {
        m_formattedMessage = formattedMessage;
    }
    inline bool isFormatted() const { return !m_formattedMessage.isNull(); }

    // Custom attributes

    inline QVariant attribute(const QString &name) const { return m_attributes.value(name); }
    inline void setAttribute(const QString &name, const QVariant &value)
    {
        m_attributes.insert(name, value);
    }
    inline void setAttributes(const QVariantHash &attrs)
    {
        m_attributes = attrs;
    }
    inline void updateAttributes(const QVariantHash &attrs)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        m_attributes.insert(attrs);
#else
        m_attributes.unite(attrs);
#endif
    }
    inline void removeAttribute(const QString &name)
    {
        m_attributes.remove(name);
    }
    inline bool hasAttribute(const QString &name) const { return m_attributes.contains(name); }
    inline QVariantHash attributes() const { return m_attributes; }

    // All message attributes including: type, line, file, function, category,
    // time, threadId and all custom attributes
    QVariantHash allAttributes() const;

private:
    // m_context string buffers
    const QByteArray m_file;
    const QByteArray m_function;
    const QByteArray m_category;

    const QtMsgType m_type = QtDebugMsg;
    const QMessageLogContext m_context;
    const QString m_message;

    const QDateTime m_time = QDateTime::currentDateTime();
#ifndef QTLOGGER_NO_THREAD
    const qint64 m_threadId = reinterpret_cast<qint64>(QThread::currentThreadId());
#endif

    QString m_formattedMessage;
    QVariantHash m_attributes;
};

inline QString qtMsgTypeToString(QtMsgType type, const QString &a_default = QStringLiteral("debug"))
{
    static const auto map = QHash<QtMsgType, QString> {
        { QtDebugMsg, QStringLiteral("debug") },
        { QtInfoMsg, QStringLiteral("info") },
        { QtWarningMsg, QStringLiteral("warning") },
        { QtCriticalMsg, QStringLiteral("critical") },
        { QtFatalMsg, QStringLiteral("fatal") },
    };
    return map.value(type, a_default);
}

inline QtMsgType stringToQtMsgType(const QString &str, QtMsgType a_default= QtDebugMsg)
{
    static const auto map = QHash<QString, QtMsgType> {
        { QStringLiteral("debug"), QtDebugMsg },
        { QStringLiteral("info"), QtInfoMsg },
        { QStringLiteral("warning"), QtWarningMsg },
        { QStringLiteral("critical"), QtCriticalMsg },
        { QStringLiteral("fatal"), QtFatalMsg },
    };
    return map.value(str, a_default);
}

inline QVariantHash LogMessage::allAttributes() const
{
    auto attrs = QVariantHash {
        { QStringLiteral("type"), qtMsgTypeToString(m_type) },
        { QStringLiteral("line"), m_context.line },
        { QStringLiteral("file"), m_context.file },
        { QStringLiteral("function"), m_context.function },
        { QStringLiteral("category"), m_context.category },
        { QStringLiteral("time"), m_time },
#ifndef QTLOGGER_NO_THREAD
        { QStringLiteral("threadId"), m_threadId },
#endif
    };

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    attrs.insert(m_attributes);
#else
    attrs.unite(m_attributes);
#endif

    return attrs;
}

} // namespace QtLogger

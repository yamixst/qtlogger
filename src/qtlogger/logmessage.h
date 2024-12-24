// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QDateTime>
#include <QHash>
#include <QThread>
#include <qlogging.h>

#include "logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT LogMessage
{
public:
    constexpr LogMessage() noexcept = default;

    LogMessage(QtMsgType type, const QMessageLogContext &context, const QString &message) noexcept
        : m_type(type),
          m_context(context.file, context.line, context.function, context.category),
          m_message(message)
    {
    }

    LogMessage(const LogMessage &logMsg) noexcept
        : m_file(logMsg.m_context.file),
          m_function(logMsg.m_context.function),
          m_category(logMsg.m_context.category),
          m_type(logMsg.m_type),
          m_context(m_file.constData(), logMsg.m_context.line, m_function.constData(),
                    m_category.constData()),
          m_message(logMsg.m_message),
          m_time(logMsg.m_time),
          m_threadId(logMsg.m_threadId),
          m_formattedMessage(logMsg.m_formattedMessage),
          m_attributes(logMsg.m_attributes)
    {
    }

    QtMsgType type() const;
    const QMessageLogContext &context() const;
    QString message() const;

    int line() const;
    const char *file() const;
    const char *function() const;
    const char *category() const;

    inline QDateTime time() const { return m_time; }
    inline qintptr threadId() const { return reinterpret_cast<qintptr>(m_threadId); }

    QString formattedMessage() const;
    void setFormattedMessage(const QString &formattedMessage);
    bool isFormatted() const;

    // Custom attributes
    QVariant attribute(const QByteArray &name) const;
    void setAttribute(const QByteArray &name, const QVariant &value);
    bool hasAttribute(const QByteArray &name) const;

private:
    // context buffers
    const QByteArray m_file;
    const QByteArray m_function;
    const QByteArray m_category;

    const QtMsgType m_type = QtDebugMsg;
    const QMessageLogContext m_context;
    const QString m_message;

    const QDateTime m_time = QDateTime::currentDateTime();
    const Qt::HANDLE m_threadId = QThread::currentThreadId();

    QString m_formattedMessage;
    QHash<QByteArray, QVariant> m_attributes;
};

inline QtMsgType LogMessage::type() const
{
    return m_type;
}

inline const QMessageLogContext &LogMessage::context() const
{
    return m_context;
}

inline QString LogMessage::message() const
{
    return m_message;
}

inline int LogMessage::line() const
{
    return m_context.line;
}

inline const char *LogMessage::file() const
{
    return m_context.file;
}

inline const char *LogMessage::function() const
{
    return m_context.function;
}

inline const char *LogMessage::category() const
{
    return m_context.category;
}

inline QString LogMessage::formattedMessage() const
{
    return isFormatted() ? m_formattedMessage : m_message;
}

inline void LogMessage::setFormattedMessage(const QString &formattedMessage)
{
    m_formattedMessage = formattedMessage;
}

inline bool LogMessage::isFormatted() const
{
    return !m_formattedMessage.isNull();
}

inline QVariant LogMessage::attribute(const QByteArray &name) const
{
    return m_attributes.value(name);
}

inline void LogMessage::setAttribute(const QByteArray &name, const QVariant &value)
{
    m_attributes.insert(name, value);
}

inline bool LogMessage::hasAttribute(const QByteArray &name) const
{
    return m_attributes.contains(name);
}

} // namespace QtLogger

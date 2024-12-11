// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QDateTime>
#include <QHash>
#include <QThread>
#include <qlogging.h>

#include "logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT DebugMessage
{
public:
    constexpr DebugMessage() noexcept = default;

    DebugMessage(QtMsgType type, const QMessageLogContext &context, const QString &message) noexcept
        : m_type(type),
          m_context(context.file, context.line, context.function, context.category),
          m_message(message)
    {
    }

    DebugMessage(const DebugMessage &dmesg) noexcept
        : m_file(dmesg.m_context.file),
          m_function(dmesg.m_context.function),
          m_category(dmesg.m_context.category),
          m_type(dmesg.m_type),
          m_context(m_file.constData(), dmesg.m_context.line, m_function.constData(),
                    m_category.constData()),
          m_message(dmesg.m_message),
          m_time(dmesg.m_time),
          m_threadId(dmesg.m_threadId),
          m_formattedMessage(dmesg.m_formattedMessage),
          m_metadata(dmesg.m_metadata)
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

    QVariant metadata(const QByteArray &name) const;
    void setMetadata(const QByteArray &name, const QVariant &value);
    bool hasMetadata(const QByteArray &name) const;

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
    QHash<QByteArray, QVariant> m_metadata;
};

inline QtMsgType DebugMessage::type() const
{
    return m_type;
}

inline const QMessageLogContext &DebugMessage::context() const
{
    return m_context;
}

inline QString DebugMessage::message() const
{
    return m_message;
}

inline int DebugMessage::line() const
{
    return m_context.line;
}

inline const char *DebugMessage::file() const
{
    return m_context.file;
}

inline const char *DebugMessage::function() const
{
    return m_context.function;
}

inline const char *DebugMessage::category() const
{
    return m_context.category;
}

inline QString DebugMessage::formattedMessage() const
{
    return isFormatted() ? m_formattedMessage : m_message;
}

inline void DebugMessage::setFormattedMessage(const QString &formattedMessage)
{
    m_formattedMessage = formattedMessage;
}

inline bool DebugMessage::isFormatted() const
{
    return !m_formattedMessage.isNull();
}

inline QVariant DebugMessage::metadata(const QByteArray &name) const
{
    return m_metadata.value(name);
}

inline void DebugMessage::setMetadata(const QByteArray &name, const QVariant &value)
{
    m_metadata.insert(name, value);
}

inline bool DebugMessage::hasMetadata(const QByteArray &name) const
{
    return m_metadata.contains(name);
}

} // namespace QtLogger

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QThread>
#include <qlogging.h>

#include "logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT DebugMessage
{
public:
    DebugMessage() = default;
    DebugMessage(QtMsgType type, const QMessageLogContext &context, const QString &message);
    DebugMessage(const DebugMessage &dmesg);

    QtMsgType type() const;
    const QMessageLogContext &context() const;
    QString message() const;

    int line() const;
    const char *file() const;
    const char *function() const;
    const char *category() const;

    inline QDateTime time() const { return m_time; }
    inline qint64 threadId() const { return reinterpret_cast<qint64>(m_threadId); }

    QString formattedMessage() const;
    void setFormattedMessage(const QString &formattedMessage);
    bool isFormatted() const;

private:
    QtMsgType m_type = QtDebugMsg;
    QMessageLogContext m_context;
    QString m_message;

    // context buffers
    QByteArray m_file;
    QByteArray m_function;
    QByteArray m_category;

    QDateTime m_time = QDateTime::currentDateTime();
    Qt::HANDLE m_threadId = QThread::currentThreadId();

    QString m_formattedMessage;
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

}

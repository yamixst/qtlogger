// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "debugmessage.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
DebugMessage::DebugMessage(QtMsgType type, const QMessageLogContext &context,
                           const QString &message)
    : m_type(type), m_message(message)
{
    m_context.version = context.version;
    m_context.line = context.line;
    m_context.file = context.file;
    m_context.function = context.function;
    m_context.category = context.category;
}

QTLOGGER_DECL_SPEC
DebugMessage::DebugMessage(const DebugMessage &dmesg)
{
    m_file = QByteArray(dmesg.m_context.file);
    m_function = QByteArray(dmesg.m_context.function);
    m_category = QByteArray(dmesg.m_context.category);

    m_type = dmesg.m_type;
    m_context.version = dmesg.m_context.version;
    m_context.line = dmesg.m_context.line;
    m_context.file = m_file.constData();
    m_context.function = m_function.constData();
    m_context.category = m_category.constData();
    m_message = dmesg.m_message;

    m_time = dmesg.m_time;
    m_threadId = dmesg.m_threadId;

    m_formattedMessage = dmesg.m_formattedMessage;
}

}

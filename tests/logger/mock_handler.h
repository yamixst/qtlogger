// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>
#include <QStringList>
#include <QVariantHash>

#include "qtlogger/handler.h"
#include "qtlogger/logmessage.h"

namespace QtLogger {

class MockHandler : public Handler
{
public:
    MockHandler() = default;
    explicit MockHandler(bool returnValue) : m_returnValue(returnValue) {}

    HandlerType type() const override { return HandlerType::Handler; }

    bool process(LogMessage &lmsg) override
    {
        m_processCallCount++;
        m_lastMessage = lmsg.message();
        m_lastFormattedMessage = lmsg.formattedMessage();
        m_lastAttributes = lmsg.attributes();
        m_lastType = lmsg.type();
        // Don't copy QMessageLogContext as it's not copyable
        
        // Store all messages for verification
        m_processedMessages.append(lmsg.message());
        m_processedTypes.append(lmsg.type());
        
        return m_returnValue;
    }

    // Test helper methods
    int processCallCount() const { return m_processCallCount; }
    QString lastMessage() const { return m_lastMessage; }
    QString lastFormattedMessage() const { return m_lastFormattedMessage; }
    QVariantHash lastAttributes() const { return m_lastAttributes; }
    QtMsgType lastType() const { return m_lastType; }
    // lastContext() removed - QMessageLogContext is not copyable
    QStringList processedMessages() const { return m_processedMessages; }
    QList<QtMsgType> processedTypes() const { return m_processedTypes; }

    void reset()
    {
        m_processCallCount = 0;
        m_lastMessage.clear();
        m_lastFormattedMessage.clear();
        m_lastAttributes.clear();
        m_lastType = QtDebugMsg;
        m_processedMessages.clear();
        m_processedTypes.clear();
    }

    void setReturnValue(bool value) { m_returnValue = value; }

private:
    bool m_returnValue = true;
    int m_processCallCount = 0;
    QString m_lastMessage;
    QString m_lastFormattedMessage;
    QVariantHash m_lastAttributes;
    QtMsgType m_lastType = QtDebugMsg;
    // m_lastContext removed - QMessageLogContext is not copyable
    QStringList m_processedMessages;
    QList<QtMsgType> m_processedTypes;
};

using MockHandlerPtr = QSharedPointer<MockHandler>;

} // namespace QtLogger
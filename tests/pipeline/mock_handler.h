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
        
        // Store all messages for verification
        m_processedMessages.append(lmsg.message());
        
        // Optionally modify the message for testing scoped behavior
        if (m_modifyMessage) {
            lmsg.setFormattedMessage(m_modifiedMessage);
        }
        
        if (m_modifyAttributes) {
            lmsg.setAttribute(m_attributeKey, m_attributeValue);
        }
        
        return m_returnValue;
    }

    // Test helper methods
    int processCallCount() const { return m_processCallCount; }
    QString lastMessage() const { return m_lastMessage; }
    QString lastFormattedMessage() const { return m_lastFormattedMessage; }
    QVariantHash lastAttributes() const { return m_lastAttributes; }
    QtMsgType lastType() const { return m_lastType; }
    QStringList processedMessages() const { return m_processedMessages; }

    void reset()
    {
        m_processCallCount = 0;
        m_lastMessage.clear();
        m_lastFormattedMessage.clear();
        m_lastAttributes.clear();
        m_processedMessages.clear();
    }

    void setReturnValue(bool value) { m_returnValue = value; }
    
    void setMessageModification(const QString &message)
    {
        m_modifyMessage = true;
        m_modifiedMessage = message;
    }
    
    void setAttributeModification(const QString &key, const QVariant &value)
    {
        m_modifyAttributes = true;
        m_attributeKey = key;
        m_attributeValue = value;
    }

private:
    bool m_returnValue = true;
    int m_processCallCount = 0;
    QString m_lastMessage;
    QString m_lastFormattedMessage;
    QVariantHash m_lastAttributes;
    QtMsgType m_lastType = QtDebugMsg;
    QStringList m_processedMessages;
    
    bool m_modifyMessage = false;
    QString m_modifiedMessage;
    
    bool m_modifyAttributes = false;
    QString m_attributeKey;
    QVariant m_attributeValue;
};

using MockHandlerPtr = QSharedPointer<MockHandler>;

} // namespace QtLogger
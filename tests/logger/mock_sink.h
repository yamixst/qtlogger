// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QSharedPointer>
#include <QStringList>
#include <QVariantHash>

#include "qtlogger/sink.h"
#include "qtlogger/logmessage.h"

namespace QtLogger {

class MockSink : public Sink
{
public:
    MockSink() = default;
    explicit MockSink(bool shouldFlush) : m_flushReturnValue(shouldFlush) {}

    void send(const LogMessage &lmsg) override
    {
        m_sendCallCount++;
        m_lastMessage = lmsg.message();
        m_lastFormattedMessage = lmsg.formattedMessage();
        m_lastAttributes = lmsg.attributes();
        m_lastType = lmsg.type();
        // Don't copy QMessageLogContext as it's not copyable
        
        // Store all messages for verification
        m_sentMessages.append(lmsg.message());
        m_sentTypes.append(lmsg.type());
        m_sentFormattedMessages.append(lmsg.formattedMessage());
    }

    bool flush() override
    {
        m_flushCallCount++;
        return m_flushReturnValue;
    }

    // Test helper methods
    int sendCallCount() const { return m_sendCallCount; }
    int flushCallCount() const { return m_flushCallCount; }
    QString lastMessage() const { return m_lastMessage; }
    QString lastFormattedMessage() const { return m_lastFormattedMessage; }
    QVariantHash lastAttributes() const { return m_lastAttributes; }
    QtMsgType lastType() const { return m_lastType; }
    // lastContext() removed - QMessageLogContext is not copyable
    QStringList sentMessages() const { return m_sentMessages; }
    QStringList sentFormattedMessages() const { return m_sentFormattedMessages; }
    QList<QtMsgType> sentTypes() const { return m_sentTypes; }

    void reset()
    {
        m_sendCallCount = 0;
        m_flushCallCount = 0;
        m_lastMessage.clear();
        m_lastFormattedMessage.clear();
        m_lastAttributes.clear();
        m_lastType = QtDebugMsg;
        m_sentMessages.clear();
        m_sentFormattedMessages.clear();
        m_sentTypes.clear();
    }

    void setFlushReturnValue(bool value) { m_flushReturnValue = value; }

private:
    bool m_flushReturnValue = true;
    int m_sendCallCount = 0;
    int m_flushCallCount = 0;
    QString m_lastMessage;
    QString m_lastFormattedMessage;
    QVariantHash m_lastAttributes;
    QtMsgType m_lastType = QtDebugMsg;
    // m_lastContext removed - QMessageLogContext is not copyable
    QStringList m_sentMessages;
    QStringList m_sentFormattedMessages;
    QList<QtMsgType> m_sentTypes;
};

using MockSinkPtr = QSharedPointer<MockSink>;

} // namespace QtLogger
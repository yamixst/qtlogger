// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>
#include <QStringList>
#include <QVariantHash>
#include <QTemporaryFile>
#include <QTextStream>
#include <QDebug>

#include "qtlogger/handler.h"
#include "qtlogger/logmessage.h"
#include "qtlogger/sink.h"
#include "qtlogger/filter.h"
#include "qtlogger/formatter.h"

namespace QtLogger {

// Mock Sink for testing output
class MockSink : public Sink
{
public:
    MockSink() = default;

    void send(const LogMessage &lmsg) override
    {
        m_processCallCount++;
        m_lastMessage = lmsg.formattedMessage();
        m_lastType = lmsg.type();
        m_allMessages.append(lmsg.formattedMessage());
        m_allTypes.append(lmsg.type());
    }

    bool flush() override
    {
        m_flushCallCount++;
        return true;
    }

    // Test helper methods
    int processCallCount() const { return m_processCallCount; }
    int flushCallCount() const { return m_flushCallCount; }
    QString lastMessage() const { return m_lastMessage; }
    QtMsgType lastType() const { return m_lastType; }
    QStringList allMessages() const { return m_allMessages; }
    QList<QtMsgType> allTypes() const { return m_allTypes; }

    void reset()
    {
        m_processCallCount = 0;
        m_flushCallCount = 0;
        m_lastMessage.clear();
        m_allMessages.clear();
        m_allTypes.clear();
    }

private:
    int m_processCallCount = 0;
    int m_flushCallCount = 0;
    QString m_lastMessage;
    QtMsgType m_lastType = QtDebugMsg;
    QStringList m_allMessages;
    QList<QtMsgType> m_allTypes;
};

using MockSinkPtr = QSharedPointer<MockSink>;

// Mock Filter for testing filtering
class MockFilter : public Filter
{
public:
    explicit MockFilter(bool shouldPass = true) : m_shouldPass(shouldPass) {}

    bool filter(const LogMessage &lmsg) override
    {
        m_processCallCount++;
        m_lastMessage = lmsg.message();
        m_processedMessages.append(lmsg.message());
        return m_shouldPass;
    }

    // Test helper methods
    int processCallCount() const { return m_processCallCount; }
    QString lastMessage() const { return m_lastMessage; }
    QStringList processedMessages() const { return m_processedMessages; }
    
    void setShouldPass(bool shouldPass) { m_shouldPass = shouldPass; }
    
    void reset()
    {
        m_processCallCount = 0;
        m_lastMessage.clear();
        m_processedMessages.clear();
    }

private:
    bool m_shouldPass = true;
    int m_processCallCount = 0;
    QString m_lastMessage;
    QStringList m_processedMessages;
};

using MockFilterPtr = QSharedPointer<MockFilter>;

// Mock Formatter for testing formatting
class MockFormatter : public Formatter
{
public:
    explicit MockFormatter(const QString &prefix = "FORMATTED: ") : m_prefix(prefix) {}

    QString format(const LogMessage &lmsg) override
    {
        m_processCallCount++;
        m_lastMessage = lmsg.message();
        m_processedMessages.append(lmsg.message());
        
        return m_prefix + lmsg.message();
    }

    // Test helper methods
    int processCallCount() const { return m_processCallCount; }
    QString lastMessage() const { return m_lastMessage; }
    QStringList processedMessages() const { return m_processedMessages; }
    
    void setPrefix(const QString &prefix) { m_prefix = prefix; }
    
    void reset()
    {
        m_processCallCount = 0;
        m_lastMessage.clear();
        m_processedMessages.clear();
    }

private:
    QString m_prefix;
    int m_processCallCount = 0;
    QString m_lastMessage;
    QStringList m_processedMessages;
};

using MockFormatterPtr = QSharedPointer<MockFormatter>;

// Mock AttrHandler for testing attribute handlers
class MockAttrHandler : public Handler
{
public:
    explicit MockAttrHandler(const QString &attrName = "test_attr", const QVariant &attrValue = "test_value")
        : m_attrName(attrName), m_attrValue(attrValue) {}

    HandlerType type() const override { return HandlerType::AttrHandler; }

    bool process(LogMessage &lmsg) override
    {
        m_processCallCount++;
        lmsg.setAttribute(m_attrName, m_attrValue);
        return true;
    }

    // Test helper methods
    int processCallCount() const { return m_processCallCount; }
    
    void setAttribute(const QString &name, const QVariant &value)
    {
        m_attrName = name;
        m_attrValue = value;
    }
    
    void reset()
    {
        m_processCallCount = 0;
    }

private:
    QString m_attrName;
    QVariant m_attrValue;
    int m_processCallCount = 0;
};

using MockAttrHandlerPtr = QSharedPointer<MockAttrHandler>;

// Helper class to capture file output
class FileOutputCapture
{
public:
    FileOutputCapture() 
    {
        m_tempFile = new QTemporaryFile();
        m_tempFile->open();
    }
    
    ~FileOutputCapture()
    {
        delete m_tempFile;
    }
    
    QString fileName() const { return m_tempFile->fileName(); }
    
    QString readAll()
    {
        m_tempFile->flush();
        m_tempFile->seek(0);
        QTextStream stream(m_tempFile);
        return stream.readAll();
    }
    
    QStringList readLines()
    {
        QString content = readAll();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        return content.split('\n', Qt::SkipEmptyParts);
#else
        return content.split('\n', QString::SkipEmptyParts);
#endif
    }

private:
    QTemporaryFile *m_tempFile;
};

} // namespace QtLogger
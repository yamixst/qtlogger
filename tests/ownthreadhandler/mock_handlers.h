// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>
#include <QStringList>
#include <QVariantHash>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>

#include "qtlogger/handler.h"
#include "qtlogger/logmessage.h"
#include "qtlogger/pipeline.h"

namespace QtLogger {

// Thread-safe mock handler for testing OwnThreadHandler
// Note: This inherits directly from Handler (not Sink/Filter) to avoid final method conflicts
class ThreadSafeMockHandler : public Handler
{
public:
    ThreadSafeMockHandler() = default;

    HandlerType type() const override { return HandlerType::Handler; }

    bool process(LogMessage &lmsg) override
    {
        QMutexLocker locker(&m_mutex);
        
        m_processCallCount++;
        m_lastMessage = lmsg.message();
        m_lastFormattedMessage = lmsg.formattedMessage();
        m_lastType = lmsg.type();
        m_processedMessages.append(lmsg.message());
        m_processingThreadIds.append(reinterpret_cast<quintptr>(QThread::currentThreadId()));
        
        // Optional delay for testing timing
        if (m_processDelay > 0) {
            QThread::msleep(m_processDelay);
        }
        
        // Signal that processing occurred
        m_waitCondition.wakeAll();
        
        return m_returnValue;
    }

    // Test helper methods (thread-safe)
    int processCallCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_processCallCount;
    }
    
    QString lastMessage() const
    {
        QMutexLocker locker(&m_mutex);
        return m_lastMessage;
    }
    
    QString lastFormattedMessage() const
    {
        QMutexLocker locker(&m_mutex);
        return m_lastFormattedMessage;
    }
    
    QtMsgType lastType() const
    {
        QMutexLocker locker(&m_mutex);
        return m_lastType;
    }
    
    QStringList processedMessages() const
    {
        QMutexLocker locker(&m_mutex);
        return m_processedMessages;
    }
    
    QList<quintptr> processingThreadIds() const
    {
        QMutexLocker locker(&m_mutex);
        return m_processingThreadIds;
    }
    
    quintptr lastProcessingThreadId() const
    {
        QMutexLocker locker(&m_mutex);
        return m_processingThreadIds.isEmpty() ? 0 : m_processingThreadIds.last();
    }

    void reset()
    {
        QMutexLocker locker(&m_mutex);
        m_processCallCount = 0;
        m_lastMessage.clear();
        m_lastFormattedMessage.clear();
        m_processedMessages.clear();
        m_processingThreadIds.clear();
    }

    void setReturnValue(bool value)
    {
        QMutexLocker locker(&m_mutex);
        m_returnValue = value;
    }
    
    void setProcessDelay(int msec)
    {
        QMutexLocker locker(&m_mutex);
        m_processDelay = msec;
    }
    
    // Wait for processing to occur (useful for async testing)
    bool waitForProcessing(int timeout = 5000)
    {
        QMutexLocker locker(&m_mutex);
        int initialCount = m_processCallCount;
        return m_waitCondition.wait(&m_mutex, timeout) && m_processCallCount > initialCount;
    }

private:
    mutable QMutex m_mutex;
    QWaitCondition m_waitCondition;
    
    bool m_returnValue = true;
    int m_processCallCount = 0;
    int m_processDelay = 0;
    QString m_lastMessage;
    QString m_lastFormattedMessage;
    QtMsgType m_lastType = QtDebugMsg;
    QStringList m_processedMessages;
    QList<quintptr> m_processingThreadIds;
};

using ThreadSafeMockHandlerPtr = QSharedPointer<ThreadSafeMockHandler>;

// Mock handler that simulates sink behavior without inheriting from Sink
class ThreadSafeMockSinkHandler : public Handler
{
public:
    ThreadSafeMockSinkHandler() = default;

    HandlerType type() const override { return HandlerType::Sink; }

    bool process(LogMessage &lmsg) override
    {
        QMutexLocker locker(&m_mutex);
        
        // Simulate sink behavior: send the formatted message
        m_sendCallCount++;
        m_lastMessage = lmsg.formattedMessage();
        m_lastType = lmsg.type();
        m_sentMessages.append(lmsg.formattedMessage());
        m_sendingThreadIds.append(reinterpret_cast<quintptr>(QThread::currentThreadId()));
        
        // Optional delay for testing timing
        if (m_sendDelay > 0) {
            QThread::msleep(m_sendDelay);
        }
        
        // Signal that sending occurred
        m_waitCondition.wakeAll();
        
        return true; // Sinks always return true
    }

    // Simulate flush method
    bool flush()
    {
        QMutexLocker locker(&m_mutex);
        m_flushCallCount++;
        m_flushingThreadIds.append(reinterpret_cast<quintptr>(QThread::currentThreadId()));
        return true;
    }

    // Test helper methods (thread-safe)
    int sendCallCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_sendCallCount;
    }
    
    int flushCallCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_flushCallCount;
    }
    
    QString lastMessage() const
    {
        QMutexLocker locker(&m_mutex);
        return m_lastMessage;
    }
    
    QtMsgType lastType() const
    {
        QMutexLocker locker(&m_mutex);
        return m_lastType;
    }
    
    QStringList sentMessages() const
    {
        QMutexLocker locker(&m_mutex);
        return m_sentMessages;
    }
    
    QList<quintptr> sendingThreadIds() const
    {
        QMutexLocker locker(&m_mutex);
        return m_sendingThreadIds;
    }
    
    QList<quintptr> flushingThreadIds() const
    {
        QMutexLocker locker(&m_mutex);
        return m_flushingThreadIds;
    }
    
    quintptr lastSendingThreadId() const
    {
        QMutexLocker locker(&m_mutex);
        return m_sendingThreadIds.isEmpty() ? 0 : m_sendingThreadIds.last();
    }

    void reset()
    {
        QMutexLocker locker(&m_mutex);
        m_sendCallCount = 0;
        m_flushCallCount = 0;
        m_lastMessage.clear();
        m_sentMessages.clear();
        m_sendingThreadIds.clear();
        m_flushingThreadIds.clear();
    }

    void setSendDelay(int msec)
    {
        QMutexLocker locker(&m_mutex);
        m_sendDelay = msec;
    }
    
    // Wait for sending to occur (useful for async testing)
    bool waitForSending(int timeout = 5000)
    {
        QMutexLocker locker(&m_mutex);
        int initialCount = m_sendCallCount;
        return m_waitCondition.wait(&m_mutex, timeout) && m_sendCallCount > initialCount;
    }

private:
    mutable QMutex m_mutex;
    QWaitCondition m_waitCondition;
    
    int m_sendCallCount = 0;
    int m_flushCallCount = 0;
    int m_sendDelay = 0;
    QString m_lastMessage;
    QtMsgType m_lastType = QtDebugMsg;
    QStringList m_sentMessages;
    QList<quintptr> m_sendingThreadIds;
    QList<quintptr> m_flushingThreadIds;
};

using ThreadSafeMockSinkHandlerPtr = QSharedPointer<ThreadSafeMockSinkHandler>;

// Mock handler that simulates filter behavior without inheriting from Filter
class ThreadSafeMockFilterHandler : public Handler
{
public:
    explicit ThreadSafeMockFilterHandler(bool shouldPass = true) : m_shouldPass(shouldPass) {}

    HandlerType type() const override { return HandlerType::Filter; }

    bool process(LogMessage &lmsg) override
    {
        QMutexLocker locker(&m_mutex);
        
        // Simulate filter behavior
        m_filterCallCount++;
        m_lastMessage = lmsg.message();
        m_filteredMessages.append(lmsg.message());
        m_filteringThreadIds.append(reinterpret_cast<quintptr>(QThread::currentThreadId()));
        
        // Optional delay for testing timing
        if (m_filterDelay > 0) {
            QThread::msleep(m_filterDelay);
        }
        
        // Signal that filtering occurred
        m_waitCondition.wakeAll();
        
        return m_shouldPass;
    }

    // Test helper methods (thread-safe)
    int filterCallCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_filterCallCount;
    }
    
    QString lastMessage() const
    {
        QMutexLocker locker(&m_mutex);
        return m_lastMessage;
    }
    
    QStringList filteredMessages() const
    {
        QMutexLocker locker(&m_mutex);
        return m_filteredMessages;
    }
    
    QList<quintptr> filteringThreadIds() const
    {
        QMutexLocker locker(&m_mutex);
        return m_filteringThreadIds;
    }
    
    quintptr lastFilteringThreadId() const
    {
        QMutexLocker locker(&m_mutex);
        return m_filteringThreadIds.isEmpty() ? 0 : m_filteringThreadIds.last();
    }

    void setShouldPass(bool shouldPass)
    {
        QMutexLocker locker(&m_mutex);
        m_shouldPass = shouldPass;
    }
    
    void setFilterDelay(int msec)
    {
        QMutexLocker locker(&m_mutex);
        m_filterDelay = msec;
    }

    void reset()
    {
        QMutexLocker locker(&m_mutex);
        m_filterCallCount = 0;
        m_lastMessage.clear();
        m_filteredMessages.clear();
        m_filteringThreadIds.clear();
    }
    
    // Wait for filtering to occur (useful for async testing)
    bool waitForFiltering(int timeout = 5000)
    {
        QMutexLocker locker(&m_mutex);
        int initialCount = m_filterCallCount;
        return m_waitCondition.wait(&m_mutex, timeout) && m_filterCallCount > initialCount;
    }

private:
    mutable QMutex m_mutex;
    QWaitCondition m_waitCondition;
    
    bool m_shouldPass = true;
    int m_filterCallCount = 0;
    int m_filterDelay = 0;
    QString m_lastMessage;
    QStringList m_filteredMessages;
    QList<quintptr> m_filteringThreadIds;
};

using ThreadSafeMockFilterHandlerPtr = QSharedPointer<ThreadSafeMockFilterHandler>;

// Mock pipeline that can be wrapped in OwnThreadHandler
class ThreadSafeMockPipeline : public Pipeline
{
public:
    explicit ThreadSafeMockPipeline(bool scoped = false) : Pipeline(scoped) {}

    // Add a mock sink to track processing
    void addMockSink(ThreadSafeMockSinkHandlerPtr sink)
    {
        m_mockSink = sink;
        append(sink);
    }

    ThreadSafeMockSinkHandlerPtr mockSink() const { return m_mockSink; }

    // Thread-safe access to processing stats
    int totalProcessCount() const
    {
        return m_mockSink ? m_mockSink->sendCallCount() : 0;
    }

private:
    ThreadSafeMockSinkHandlerPtr m_mockSink;
};

using ThreadSafeMockPipelinePtr = QSharedPointer<ThreadSafeMockPipeline>;

// Helper class for testing thread behavior
class ThreadTester
{
public:
    static quintptr mainThreadId()
    {
        return reinterpret_cast<quintptr>(QCoreApplication::instance()->thread()->currentThreadId());
    }
    
    static quintptr currentThreadId()
    {
        return reinterpret_cast<quintptr>(QThread::currentThreadId());
    }
    
    static bool isMainThread()
    {
        return QThread::currentThread() == QCoreApplication::instance()->thread();
    }
    
    static bool isDifferentThread(quintptr threadId)
    {
        return threadId != currentThreadId() && threadId != 0;
    }
    
    // Wait for a condition with timeout
    template<typename Predicate>
    static bool waitFor(Predicate predicate, int timeout = 5000)
    {
        QTimer timer;
        timer.setSingleShot(true);
        timer.start(timeout);
        
        while (!predicate() && timer.isActive()) {
            QCoreApplication::processEvents();
            QThread::msleep(10);
        }
        
        return predicate();
    }
};

} // namespace QtLogger
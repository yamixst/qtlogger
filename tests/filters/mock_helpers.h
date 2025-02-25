#pragma once

#include <QDateTime>
#include <QLoggingCategory>
#include <QString>
#include <QSharedPointer>
#include <QTimer>
#include <functional>

#include "qtlogger/logmessage.h"

namespace QtLogger {
namespace Test {

/**
 * @brief Mock helpers for comprehensive filter testing
 */
class MockHelpers
{
public:
    /**
     * @brief Creates a LogMessage with custom timestamp
     */
    static LogMessage createMessageWithTime(
        const QString& message,
        const QDateTime& timestamp,
        QtMsgType type = QtDebugMsg,
        const QString& category = "test.category",
        const QString& function = "testFunction",
        const QString& file = "test.cpp",
        int line = 42
    )
    {
        // Store strings to ensure they remain valid
        static thread_local QByteArray s_file;
        static thread_local QByteArray s_function;
        static thread_local QByteArray s_category;
        
        s_file = QByteArray(file.toUtf8());
        s_function = QByteArray(function.toUtf8());
        s_category = QByteArray(category.toUtf8());
        
        QMessageLogContext context(
            s_file.constData(),
            line,
            s_function.constData(),
            s_category.constData()
        );
        
        LogMessage msg(type, context, message);
        
        // Note: LogMessage sets time automatically in constructor
        // This is a limitation - we can't easily mock the timestamp
        // In a real implementation, you might need to modify LogMessage
        // to accept timestamp as a parameter or provide a test constructor
        
        return msg;
    }
    
    /**
     * @brief Creates a series of messages with incremental timestamps
     */
    static QList<LogMessage> createMessageSeries(
        const QStringList& messages,
        const QDateTime& startTime,
        int intervalMs = 1000,
        QtMsgType type = QtDebugMsg,
        const QString& category = "test.category"
    )
    {
        QList<LogMessage> result;
        QDateTime currentTime = startTime;
        
        for (const QString& message : messages) {
            result.append(createMessageWithTime(message, currentTime, type, category));
            currentTime = currentTime.addMSecs(intervalMs);
        }
        
        return result;
    }
    
    /**
     * @brief Creates messages with different log levels
     */
    static QList<LogMessage> createMultiLevelMessages(
        const QString& baseMessage,
        const QString& category = "test.category"
    )
    {
        QList<LogMessage> messages;
        QList<QtMsgType> types = { QtDebugMsg, QtInfoMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg };
        QStringList typeNames = { "debug", "info", "warning", "critical", "fatal" };
        
        for (int i = 0; i < types.size(); ++i) {
            QString message = QString("%1 - %2").arg(typeNames[i]).arg(baseMessage);
            messages.append(createMessageWithTime(message, QDateTime::currentDateTime(), types[i], category));
        }
        
        return messages;
    }
    
    /**
     * @brief Creates messages with different categories
     */
    static QList<LogMessage> createMultiCategoryMessages(
        const QString& message,
        const QStringList& categories,
        QtMsgType type = QtDebugMsg
    )
    {
        QList<LogMessage> messages;
        
        for (const QString& category : categories) {
            messages.append(createMessageWithTime(message, QDateTime::currentDateTime(), type, category));
        }
        
        return messages;
    }
    
    /**
     * @brief Mock filter that can be configured for testing
     */
    class MockFilter
    {
    public:
        using FilterFunction = std::function<bool(const LogMessage&)>;
        
        MockFilter(FilterFunction func = nullptr) 
            : m_function(func ? func : [](const LogMessage&) { return true; })
            , m_callCount(0)
        {
        }
        
        bool filter(const LogMessage& msg)
        {
            m_callCount++;
            m_lastMessage = msg.message();
            m_lastType = msg.type();
            m_lastCategory = QString(msg.category());
            
            return m_function(msg);
        }
        
        // Statistics and state
        int callCount() const { return m_callCount; }
        QString lastMessage() const { return m_lastMessage; }
        QtMsgType lastType() const { return m_lastType; }
        QString lastCategory() const { return m_lastCategory; }
        
        void reset() 
        {
            m_callCount = 0;
            m_lastMessage.clear();
            m_lastType = QtDebugMsg;
            m_lastCategory.clear();
        }
        
        void setFunction(FilterFunction func) { m_function = func; }
        
    private:
        FilterFunction m_function;
        int m_callCount;
        QString m_lastMessage;
        QtMsgType m_lastType;
        QString m_lastCategory;
    };
    
    /**
     * @brief Performance measurement helper
     */
    class PerformanceMeasurer
    {
    public:
        PerformanceMeasurer() : m_startTime(0), m_endTime(0) {}
        
        void start() { m_startTime = QDateTime::currentMSecsSinceEpoch(); }
        void stop() { m_endTime = QDateTime::currentMSecsSinceEpoch(); }
        
        qint64 elapsedMs() const { return m_endTime - m_startTime; }
        double messagesPerSecond(int messageCount) const 
        {
            qint64 elapsed = elapsedMs();
            return elapsed > 0 ? (messageCount * 1000.0 / elapsed) : 0.0;
        }
        
    private:
        qint64 m_startTime;
        qint64 m_endTime;
    };
    
    /**
     * @brief Creates test data for stress testing
     */
    static QList<LogMessage> createStressTestData(
        int messageCount,
        int categoryCount = 10,
        const QString& baseMessage = "Test message"
    )
    {
        QList<LogMessage> messages;
        QStringList categories;
        
        // Create category names
        for (int i = 0; i < categoryCount; ++i) {
            categories.append(QString("category%1").arg(i));
        }
        
        QList<QtMsgType> types = { QtDebugMsg, QtInfoMsg, QtWarningMsg, QtCriticalMsg };
        
        for (int i = 0; i < messageCount; ++i) {
            QString message = QString("%1 #%2").arg(baseMessage).arg(i);
            QString category = categories[i % categoryCount];
            QtMsgType type = types[i % types.size()];
            
            messages.append(createMessageWithTime(message, QDateTime::currentDateTime(), type, category));
        }
        
        return messages;
    }
    
    /**
     * @brief Validates filter behavior with expected results
     */
    template<typename FilterType>
    static bool validateFilterBehavior(
        FilterType& filter,
        const QList<LogMessage>& inputs,
        const QList<bool>& expectedResults
    )
    {
        if (inputs.size() != expectedResults.size()) {
            return false;
        }
        
        for (int i = 0; i < inputs.size(); ++i) {
            bool result = filter.filter(inputs[i]);
            if (result != expectedResults[i]) {
                return false;
            }
        }
        
        return true;
    }
    
    /**
     * @brief Creates messages with special characters for edge case testing
     */
    static QList<LogMessage> createEdgeCaseMessages()
    {
        QList<LogMessage> messages;
        
        // Empty message
        messages.append(createMessageWithTime("", QDateTime::currentDateTime()));
        
        // Very long message
        QString longMessage = QString("x").repeated(10000);
        messages.append(createMessageWithTime(longMessage, QDateTime::currentDateTime()));
        
        // Unicode characters
        messages.append(createMessageWithTime("Unicode: äöü €中文🙂", QDateTime::currentDateTime()));
        
        // Special characters
        messages.append(createMessageWithTime("Special: \n\t\r\"'\\", QDateTime::currentDateTime()));
        
        // Control characters
        messages.append(createMessageWithTime(QString("Control: \x01\x02\x03"), QDateTime::currentDateTime()));
        
        // Null character (carefully handled)
        messages.append(createMessageWithTime(QString("Null: %1").arg(QChar(0)), QDateTime::currentDateTime()));
        
        // HTML/XML content
        messages.append(createMessageWithTime("<html><body>Test &amp; content</body></html>", QDateTime::currentDateTime()));
        
        // JSON-like content
        messages.append(createMessageWithTime("{\"key\": \"value\", \"number\": 123}", QDateTime::currentDateTime()));
        
        // SQL-like content
        messages.append(createMessageWithTime("SELECT * FROM users WHERE id = 'test'; DROP TABLE users;", QDateTime::currentDateTime()));
        
        return messages;
    }
    
    /**
     * @brief Creates a chain of filters for pipeline testing
     */
    template<typename... Filters>
    static bool processMessageThroughChain(const LogMessage& msg, Filters&... filters)
    {
        bool result = true;
        auto processFilter = [&](auto& filter) {
            if (result) {
                result = filter.filter(msg);
            }
        };
        
        (processFilter(filters), ...);
        return result;
    }
};

/**
 * @brief RAII helper for measuring test execution time
 */
class ScopedTimer
{
public:
    ScopedTimer(const QString& testName) 
        : m_testName(testName)
        , m_startTime(QDateTime::currentMSecsSinceEpoch())
    {
    }
    
    ~ScopedTimer()
    {
        qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_startTime;
        qDebug() << "Test" << m_testName << "completed in" << elapsed << "ms";
    }
    
private:
    QString m_testName;
    qint64 m_startTime;
};

#define SCOPED_TIMER(name) ScopedTimer timer(name)

} // namespace Test
} // namespace QtLogger
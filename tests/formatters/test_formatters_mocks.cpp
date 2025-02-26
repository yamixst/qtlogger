#include <QtTest/QtTest>
#include <QDateTime>
#include <QThread>
#include <QVariantHash>
#include <QMessageLogContext>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTimer>

#include "qtlogger.h"
#include "mock_logmessage.h"

// Include all formatters
#include "qtlogger/formatters/qtlogmessageformatter.h"
#include "qtlogger/formatters/functionformatter.h"
#include "qtlogger/formatters/jsonformatter.h"
#include "qtlogger/formatters/patternformatter.h"
#include "qtlogger/formatters/prettyformatter.h"

using namespace QtLogger;
using namespace QtLogger::Test;

/**
 * @brief Mock class for testing external system dependencies
 */
class MockSystemInfo : public QObject
{
    Q_OBJECT

public:
    MockSystemInfo() = default;

    // Mock methods that could be external dependencies
    QString getCurrentUser() const { return m_currentUser; }
    QString getHostname() const { return m_hostname; }
    qint64 getCurrentMemoryUsage() const { return m_memoryUsage; }
    QString getSystemTime() const { return m_systemTime; }
    bool isNetworkAvailable() const { return m_networkAvailable; }

    // Setters for testing
    void setCurrentUser(const QString& user) { m_currentUser = user; }
    void setHostname(const QString& hostname) { m_hostname = hostname; }
    void setMemoryUsage(qint64 memory) { m_memoryUsage = memory; }
    void setSystemTime(const QString& time) { m_systemTime = time; }
    void setNetworkAvailable(bool available) { m_networkAvailable = available; }

signals:
    void systemInfoRequested(const QString& type);

private:
    QString m_currentUser = "testuser";
    QString m_hostname = "testhost";
    qint64 m_memoryUsage = 1024000;
    QString m_systemTime = "2024-01-15T10:30:45";
    bool m_networkAvailable = true;
};

/**
 * @brief Mock formatter that uses external system dependencies
 */
class MockSystemFormatter : public Formatter
{
public:
    MockSystemFormatter(MockSystemInfo* systemInfo) : m_systemInfo(systemInfo) {}

    QString format(const LogMessage& lmsg) override
    {
        if (m_systemInfo) {
            emit m_systemInfo->systemInfoRequested("format");
        }

        QString result;
        result += QString("[%1@%2] ").arg(
            m_systemInfo ? m_systemInfo->getCurrentUser() : "unknown",
            m_systemInfo ? m_systemInfo->getHostname() : "unknown"
        );

        result += QString("[MEM:%1KB] ").arg(
            m_systemInfo ? m_systemInfo->getCurrentMemoryUsage() / 1024 : 0
        );

        if (m_systemInfo && !m_systemInfo->isNetworkAvailable()) {
            result += "[OFFLINE] ";
        }

        result += QString("[%1] %2").arg(
            qtMsgTypeToString(lmsg.type()).toUpper(),
            lmsg.message()
        );

        return result;
    }

private:
    MockSystemInfo* m_systemInfo;
};

/**
 * @brief Mock time provider for testing time-dependent formatting
 */
class MockTimeProvider
{
public:
    static MockTimeProvider& instance()
    {
        static MockTimeProvider s_instance;
        return s_instance;
    }

    QDateTime currentDateTime() const { return m_currentTime; }
    void setCurrentDateTime(const QDateTime& time) { m_currentTime = time; }
    void advanceTime(int msecs) { m_currentTime = m_currentTime.addMSecs(msecs); }

private:
    QDateTime m_currentTime = QDateTime::fromString("2024-01-15T10:30:45", Qt::ISODate);
};

/**
 * @brief Mock formatter that uses time provider for consistent testing
 */
class MockTimeFormatter : public Formatter
{
public:
    QString format(const LogMessage& lmsg) override
    {
        auto mockTime = MockTimeProvider::instance().currentDateTime();
        return QString("[%1] %2: %3").arg(
            mockTime.toString("hh:mm:ss.zzz"),
            qtMsgTypeToString(lmsg.type()).toUpper(),
            lmsg.message()
        );
    }
};

/**
 * @brief Mock I/O handler for testing formatter output
 */
class MockIOHandler : public QObject
{
    Q_OBJECT

public:
    MockIOHandler() = default;

    void write(const QString& data)
    {
        m_writtenData.append(data);
        emit dataWritten(data);
    }

    QStringList getWrittenData() const { return m_writtenData; }
    void clearWrittenData() { m_writtenData.clear(); }
    int getWriteCount() const { return m_writtenData.size(); }

signals:
    void dataWritten(const QString& data);

private:
    QStringList m_writtenData;
};

class TestFormatterMocks : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Mock system info tests
    void testMockSystemFormatterBasic();
    void testMockSystemFormatterWithOfflineNetwork();
    void testMockSystemFormatterSignals();
    void testMockSystemFormatterDifferentUsers();
    void testMockSystemFormatterMemoryValues();

    // Mock time provider tests
    void testMockTimeFormatterConsistentTime();
    void testMockTimeFormatterTimeAdvancement();
    void testMockTimeFormatterDifferentTimes();

    // Mock I/O handler tests
    void testFormatterWithMockIO();
    void testMultipleFormattersWithMockIO();
    void testFormatterOutputCapture();

    // Integration tests with mocks
    void testFormatterChainWithMocks();
    void testFormatterPerformanceWithMocks();
    void testFormatterErrorHandlingWithMocks();

    // Mock error scenarios
    void testFormatterWithNullMockDependency();
    void testFormatterWithFailingMockDependency();
    void testFormatterWithSlowMockDependency();

private:
    MockSystemInfo* m_systemInfo;
    MockIOHandler* m_ioHandler;
};

void TestFormatterMocks::initTestCase()
{
    // Initialize mock objects
    m_systemInfo = new MockSystemInfo();
    m_ioHandler = new MockIOHandler();

    // Set up default mock values
    m_systemInfo->setCurrentUser("testuser");
    m_systemInfo->setHostname("testhost");
    m_systemInfo->setMemoryUsage(2048000); // 2MB
    m_systemInfo->setNetworkAvailable(true);
}

void TestFormatterMocks::cleanupTestCase()
{
    delete m_systemInfo;
    delete m_ioHandler;
}

void TestFormatterMocks::init()
{
    // Reset mock states before each test
    m_ioHandler->clearWrittenData();
    
    // Reset to default values
    m_systemInfo->setCurrentUser("testuser");
    m_systemInfo->setHostname("testhost");
    m_systemInfo->setMemoryUsage(2048000);
    m_systemInfo->setNetworkAvailable(true);
    
    // Reset time
    MockTimeProvider::instance().setCurrentDateTime(
        QDateTime::fromString("2024-01-15T10:30:45", Qt::ISODate)
    );
}

void TestFormatterMocks::cleanup()
{
    // Clean up after each test
    m_ioHandler->clearWrittenData();
}

// Mock system info tests

void TestFormatterMocks::testMockSystemFormatterBasic()
{
    MockSystemFormatter formatter(m_systemInfo);
    auto msg = MockLogMessage::create(QtInfoMsg, "System test message");
    
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("[testuser@testhost]"));
    QVERIFY(formatted.contains("[MEM:2000KB]"));
    QVERIFY(formatted.contains("[INFO] System test message"));
    QVERIFY(!formatted.contains("[OFFLINE]")); // Network is available
}

void TestFormatterMocks::testMockSystemFormatterWithOfflineNetwork()
{
    m_systemInfo->setNetworkAvailable(false);
    
    MockSystemFormatter formatter(m_systemInfo);
    auto msg = MockLogMessage::create(QtWarningMsg, "Offline test");
    
    QString formatted = formatter.format(msg);
    
    QVERIFY(formatted.contains("[OFFLINE]"));
    QVERIFY(formatted.contains("[WARNING] Offline test"));
}

void TestFormatterMocks::testMockSystemFormatterSignals()
{
    MockSystemFormatter formatter(m_systemInfo);
    QSignalSpy spy(m_systemInfo, &MockSystemInfo::systemInfoRequested);
    
    auto msg = MockLogMessage::create(QtDebugMsg, "Signal test");
    formatter.format(msg);
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QString("format"));
}

void TestFormatterMocks::testMockSystemFormatterDifferentUsers()
{
    MockSystemFormatter formatter(m_systemInfo);
    
    // Test with different users
    QStringList users = {"alice", "bob", "admin", "guest"};
    QStringList hosts = {"server1", "workstation", "laptop", "mobile"};
    
    for (int i = 0; i < users.size(); ++i) {
        m_systemInfo->setCurrentUser(users[i]);
        m_systemInfo->setHostname(hosts[i]);
        
        auto msg = MockLogMessage::create(QtDebugMsg, QString("Message from %1").arg(users[i]));
        QString formatted = formatter.format(msg);
        
        QString expectedUserHost = QString("[%1@%2]").arg(users[i], hosts[i]);
        QVERIFY(formatted.contains(expectedUserHost));
        QVERIFY(formatted.contains(QString("Message from %1").arg(users[i])));
    }
}

void TestFormatterMocks::testMockSystemFormatterMemoryValues()
{
    MockSystemFormatter formatter(m_systemInfo);
    
    // Test with different memory values
    QList<qint64> memoryValues = {1024, 1048576, 2097152, 5242880}; // 1KB, 1MB, 2MB, 5MB
    QStringList expectedMemStr = {"1KB", "1024KB", "2048KB", "5120KB"};
    
    for (int i = 0; i < memoryValues.size(); ++i) {
        m_systemInfo->setMemoryUsage(memoryValues[i]);
        
        auto msg = MockLogMessage::create(QtInfoMsg, "Memory test");
        QString formatted = formatter.format(msg);
        
        QVERIFY(formatted.contains(QString("[MEM:%1]").arg(expectedMemStr[i])));
    }
}

// Mock time provider tests

void TestFormatterMocks::testMockTimeFormatterConsistentTime()
{
    MockTimeFormatter formatter;
    
    // Set specific time
    QDateTime testTime = QDateTime::fromString("2024-02-20T15:45:30.123", Qt::ISODate);
    MockTimeProvider::instance().setCurrentDateTime(testTime);
    
    auto msg = MockLogMessage::create(QtDebugMsg, "Time test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(formatted.contains("[15:45:30.123]"));
    QVERIFY(formatted.contains("DEBUG"));
    QVERIFY(formatted.contains("Time test"));
    
    // Format another message - should have same time
    auto msg2 = MockLogMessage::create(QtInfoMsg, "Another time test");
    QString formatted2 = formatter.format(msg2);
    
    QVERIFY(formatted2.contains("[15:45:30.123]"));
    QVERIFY(formatted2.contains("INFO"));
    QVERIFY(formatted2.contains("Another time test"));
}

void TestFormatterMocks::testMockTimeFormatterTimeAdvancement()
{
    MockTimeFormatter formatter;
    
    QDateTime startTime = QDateTime::fromString("2024-01-01T12:00:00.000", Qt::ISODate);
    MockTimeProvider::instance().setCurrentDateTime(startTime);
    
    auto msg1 = MockLogMessage::create(QtDebugMsg, "First message");
    QString formatted1 = formatter.format(msg1);
    QVERIFY(formatted1.contains("[12:00:00.000]"));
    
    // Advance time by 1.5 seconds
    MockTimeProvider::instance().advanceTime(1500);
    
    auto msg2 = MockLogMessage::create(QtInfoMsg, "Second message");
    QString formatted2 = formatter.format(msg2);
    QVERIFY(formatted2.contains("[12:00:01.500]"));
    
    // Advance time by 5 minutes
    MockTimeProvider::instance().advanceTime(5 * 60 * 1000);
    
    auto msg3 = MockLogMessage::create(QtWarningMsg, "Third message");
    QString formatted3 = formatter.format(msg3);
    QVERIFY(formatted3.contains("[12:05:01.500]"));
}

void TestFormatterMocks::testMockTimeFormatterDifferentTimes()
{
    MockTimeFormatter formatter;
    
    QList<QDateTime> testTimes = {
        QDateTime::fromString("2024-01-01T00:00:00.000", Qt::ISODate),
        QDateTime::fromString("2024-06-15T12:30:45.123", Qt::ISODate),
        QDateTime::fromString("2024-12-31T23:59:59.999", Qt::ISODate)
    };
    
    QStringList expectedTimeStrings = {
        "00:00:00.000",
        "12:30:45.123", 
        "23:59:59.999"
    };
    
    for (int i = 0; i < testTimes.size(); ++i) {
        MockTimeProvider::instance().setCurrentDateTime(testTimes[i]);
        
        auto msg = MockLogMessage::create(QtDebugMsg, QString("Message %1").arg(i));
        QString formatted = formatter.format(msg);
        
        QVERIFY(formatted.contains(QString("[%1]").arg(expectedTimeStrings[i])));
    }
}

// Mock I/O handler tests

void TestFormatterMocks::testFormatterWithMockIO()
{
    auto jsonFormatter = JsonFormatter::instance();
    QSignalSpy ioSpy(m_ioHandler, &MockIOHandler::dataWritten);
    
    auto msg = MockLogMessage::create(QtInfoMsg, "IO test message");
    QString formatted = jsonFormatter->format(msg);
    
    // Simulate writing to mock I/O
    m_ioHandler->write(formatted);
    
    QCOMPARE(ioSpy.count(), 1);
    QCOMPARE(m_ioHandler->getWriteCount(), 1);
    
    QStringList writtenData = m_ioHandler->getWrittenData();
    QCOMPARE(writtenData.size(), 1);
    QVERIFY(writtenData[0].contains("IO test message"));
    
    // Verify it's valid JSON
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(writtenData[0].toUtf8(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
}

void TestFormatterMocks::testMultipleFormattersWithMockIO()
{
    auto jsonFormatter = JsonFormatter::instance();
    auto prettyFormatter = PrettyFormatter::instance();
    MockTimeFormatter timeFormatter;
    
    QSignalSpy ioSpy(m_ioHandler, &MockIOHandler::dataWritten);
    
    auto msg = MockLogMessage::create(QtWarningMsg, "Multiple formatter test");
    
    // Format with different formatters and write to mock I/O
    QString jsonFormatted = jsonFormatter->format(msg);
    QString prettyFormatted = prettyFormatter->format(msg);
    QString timeFormatted = timeFormatter.format(msg);
    
    m_ioHandler->write(jsonFormatted);
    m_ioHandler->write(prettyFormatted);
    m_ioHandler->write(timeFormatted);
    
    QCOMPARE(ioSpy.count(), 3);
    QCOMPARE(m_ioHandler->getWriteCount(), 3);
    
    QStringList writtenData = m_ioHandler->getWrittenData();
    QVERIFY(writtenData[0].contains("Multiple formatter test")); // JSON
    QVERIFY(writtenData[1].contains("Multiple formatter test")); // Pretty
    QVERIFY(writtenData[2].contains("Multiple formatter test")); // Time
    QVERIFY(writtenData[2].contains("WARNING")); // Time formatter includes type
}

void TestFormatterMocks::testFormatterOutputCapture()
{
    FunctionFormatter customFormatter([](const LogMessage& msg) {
        return QString("CUSTOM[%1]: %2 at %3")
            .arg(qtMsgTypeToString(msg.type()))
            .arg(msg.message())
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    });
    
    QSignalSpy ioSpy(m_ioHandler, &MockIOHandler::dataWritten);
    
    // Process multiple messages
    QStringList testMessages = {
        "First custom message",
        "Second custom message", 
        "Third custom message"
    };
    
    QList<QtMsgType> testTypes = {QtDebugMsg, QtInfoMsg, QtWarningMsg};
    
    for (int i = 0; i < testMessages.size(); ++i) {
        auto msg = MockLogMessage::create(testTypes[i], testMessages[i]);
        QString formatted = customFormatter.format(msg);
        m_ioHandler->write(formatted);
    }
    
    QCOMPARE(ioSpy.count(), 3);
    QCOMPARE(m_ioHandler->getWriteCount(), 3);
    
    QStringList writtenData = m_ioHandler->getWrittenData();
    for (int i = 0; i < testMessages.size(); ++i) {
        QVERIFY(writtenData[i].startsWith("CUSTOM["));
        QVERIFY(writtenData[i].contains(testMessages[i]));
        QVERIFY(writtenData[i].contains(qtMsgTypeToString(testTypes[i])));
    }
}

// Integration tests with mocks

void TestFormatterMocks::testFormatterChainWithMocks()
{
    // Test formatter chain using mocks to simulate complex scenarios
    MockSystemFormatter systemFormatter(m_systemInfo);
    MockTimeFormatter timeFormatter;
    auto jsonFormatter = JsonFormatter::instance();
    
    QSignalSpy systemSpy(m_systemInfo, &MockSystemInfo::systemInfoRequested);
    QSignalSpy ioSpy(m_ioHandler, &MockIOHandler::dataWritten);
    
    // Set up mock environment
    m_systemInfo->setCurrentUser("chainuser");
    m_systemInfo->setHostname("chainhost");
    m_systemInfo->setMemoryUsage(4194304); // 4MB
    MockTimeProvider::instance().setCurrentDateTime(
        QDateTime::fromString("2024-03-10T14:25:30.500", Qt::ISODate)
    );
    
    auto msg = MockLogMessage::create(QtCriticalMsg, "Chain test message");
    
    // Process through formatter chain
    QString systemFormatted = systemFormatter.format(msg);
    QString timeFormatted = timeFormatter.format(msg);
    QString jsonFormatted = jsonFormatter->format(msg);
    
    // Write all outputs
    m_ioHandler->write(systemFormatted);
    m_ioHandler->write(timeFormatted);
    m_ioHandler->write(jsonFormatted);
    
    // Verify system formatter
    QVERIFY(systemFormatted.contains("[chainuser@chainhost]"));
    QVERIFY(systemFormatted.contains("[MEM:4096KB]"));
    QVERIFY(systemFormatted.contains("[CRITICAL] Chain test message"));
    QCOMPARE(systemSpy.count(), 1);
    
    // Verify time formatter
    QVERIFY(timeFormatted.contains("[14:25:30.500]"));
    QVERIFY(timeFormatted.contains("CRITICAL"));
    QVERIFY(timeFormatted.contains("Chain test message"));
    
    // Verify JSON formatter
    QJsonDocument doc = QJsonDocument::fromJson(jsonFormatted.toUtf8());
    QJsonObject obj = doc.object();
    QCOMPARE(obj["message"].toString(), QString("Chain test message"));
    QCOMPARE(obj["type"].toString(), QString("critical"));
    
    // Verify I/O capture
    QCOMPARE(ioSpy.count(), 3);
    QCOMPARE(m_ioHandler->getWriteCount(), 3);
}

void TestFormatterMocks::testFormatterPerformanceWithMocks()
{
    MockSystemFormatter formatter(m_systemInfo);
    QSignalSpy systemSpy(m_systemInfo, &MockSystemInfo::systemInfoRequested);
    
    // Test performance with many messages
    const int messageCount = 100;
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < messageCount; ++i) {
        auto msg = MockLogMessage::create(QtDebugMsg, QString("Performance test message %1").arg(i));
        QString formatted = formatter.format(msg);
        m_ioHandler->write(formatted);
    }
    
    qint64 elapsed = timer.elapsed();
    
    // Verify all messages were processed
    QCOMPARE(systemSpy.count(), messageCount);
    QCOMPARE(m_ioHandler->getWriteCount(), messageCount);
    
    // Performance should be reasonable (less than 1 second for 100 messages)
    QVERIFY(elapsed < 1000);
    
    // Verify some random messages
    QStringList writtenData = m_ioHandler->getWrittenData();
    QVERIFY(writtenData[0].contains("Performance test message 0"));
    QVERIFY(writtenData[50].contains("Performance test message 50"));
    QVERIFY(writtenData[99].contains("Performance test message 99"));
}

void TestFormatterMocks::testFormatterErrorHandlingWithMocks()
{
    // Test how formatters handle mock errors
    MockSystemFormatter formatter(nullptr); // Null mock dependency
    
    auto msg = MockLogMessage::create(QtCriticalMsg, "Error handling test");
    QString formatted = formatter.format(msg);
    
    // Should handle null dependency gracefully
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("[unknown@unknown]"));
    QVERIFY(formatted.contains("[MEM:0KB]"));
    QVERIFY(formatted.contains("[CRITICAL] Error handling test"));
}

// Mock error scenarios

void TestFormatterMocks::testFormatterWithNullMockDependency()
{
    MockSystemFormatter formatter(nullptr);
    
    auto msg1 = MockLogMessage::create(QtDebugMsg, "Null dependency test 1");
    auto msg2 = MockLogMessage::create(QtWarningMsg, "Null dependency test 2");
    
    QString formatted1 = formatter.format(msg1);
    QString formatted2 = formatter.format(msg2);
    
    // Should not crash and provide fallback values
    QVERIFY(formatted1.contains("unknown@unknown"));
    QVERIFY(formatted1.contains("MEM:0KB"));
    QVERIFY(formatted1.contains("Null dependency test 1"));
    
    QVERIFY(formatted2.contains("unknown@unknown"));
    QVERIFY(formatted2.contains("MEM:0KB"));
    QVERIFY(formatted2.contains("Null dependency test 2"));
}

void TestFormatterMocks::testFormatterWithFailingMockDependency()
{
    // Simulate failing dependency by setting extreme values
    m_systemInfo->setCurrentUser(""); // Empty user
    m_systemInfo->setHostname(""); // Empty hostname
    m_systemInfo->setMemoryUsage(-1); // Invalid memory
    
    MockSystemFormatter formatter(m_systemInfo);
    auto msg = MockLogMessage::create(QtCriticalMsg, "Failing dependency test");
    
    QString formatted = formatter.format(msg);
    
    // Should handle invalid values gracefully
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("[@]")); // Empty user@hostname
    QVERIFY(formatted.contains("Failing dependency test"));
    
    // Memory should be handled (negative value divided by 1024)
    QVERIFY(formatted.contains("MEM:")); // Should not crash
}

void TestFormatterMocks::testFormatterWithSlowMockDependency()
{
    // Create a custom formatter that simulates slow operations
    class SlowMockFormatter : public Formatter
    {
    public:
        SlowMockFormatter(int delayMs) : m_delayMs(delayMs) {}
        
        QString format(const LogMessage& lmsg) override
        {
            // Simulate slow operation
            QThread::msleep(m_delayMs);
            return QString("SLOW[%1ms]: %2").arg(m_delayMs).arg(lmsg.message());
        }
        
    private:
        int m_delayMs;
    };
    
    SlowMockFormatter slowFormatter(10); // 10ms delay
    QElapsedTimer timer;
    
    auto msg = MockLogMessage::create(QtInfoMsg, "Slow dependency test");
    
    timer.start();
    QString formatted = slowFormatter.format(msg);
    qint64 elapsed = timer.elapsed();
    
    // Should take at least the delay time
    QVERIFY(elapsed >= 10);
    QVERIFY(formatted.contains("SLOW[10ms]: Slow dependency test"));
}

QTEST_MAIN(TestFormatterMocks)
#include "test_formatters_mocks.moc"
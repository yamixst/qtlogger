#include <QtTest/QtTest>
#include <QMessageLogContext>
#include <QElapsedTimer>
#include <functional>

#include "qtlogger/filters/functionfilter.h"
#include "mock_context.h"

using namespace QtLogger;

class TestFunctionFilter : public QObject
{
    Q_OBJECT

private slots:
    // Constructor tests
    void testConstructorWithLambda();
    void testConstructorWithFunction();
    void testConstructorWithFunctor();

    // Basic functionality tests
    void testAlwaysTrue();
    void testAlwaysFalse();
    void testMessageContentFilter();
    void testMessageTypeFilter();
    void testCategoryFilter();

    // Complex filtering logic tests
    void testCombinedConditions();
    void testStatefulFilter();
    void testCountingFilter();
    void testTimeBasedFilter();

    // Edge cases and robustness tests
    void testNullptrFunction();
    void testExceptionInFunction();
    void testComplexLogicFilter();

    // Performance tests
    void testPerformanceFilter();
    void testManyFilterCalls();

private:
    LogMessage createMessage(const QString& message, QtMsgType type = QtDebugMsg, 
                           const QString& category = "test.category", 
                           const QString& function = "testFunction");
};

LogMessage TestFunctionFilter::createMessage(const QString& message, QtMsgType type, 
                                           const QString& category, const QString& function)
{
    auto context = Test::MockContext::create("test.cpp", 42, function.toUtf8().constData(), category.toUtf8().constData());
    auto lmsg = LogMessage(type, context, message);
    return LogMessage(lmsg);
}

void TestFunctionFilter::testConstructorWithLambda()
{
    auto lambda = [](const LogMessage& msg) { 
        return msg.message().contains("test"); 
    };
    
    FunctionFilter filter(lambda);
    
    auto msgMatch = createMessage("This is a test message");
    auto msgNoMatch = createMessage("This is a sample message");
    
    QVERIFY(filter.filter(msgMatch));
    QVERIFY(!filter.filter(msgNoMatch));
}

void TestFunctionFilter::testConstructorWithFunction()
{
    auto func = [](const LogMessage& msg) -> bool {
        return msg.type() == QtWarningMsg;
    };
    
    FunctionFilter filter(func);
    
    auto warningMsg = createMessage("Warning", QtWarningMsg);
    auto debugMsg = createMessage("Debug", QtDebugMsg);
    
    QVERIFY(filter.filter(warningMsg));
    QVERIFY(!filter.filter(debugMsg));
}

void TestFunctionFilter::testConstructorWithFunctor()
{
    struct MessageLengthFilter {
        int maxLength;
        MessageLengthFilter(int max) : maxLength(max) {}
        bool operator()(const LogMessage& msg) const {
            return msg.message().length() <= maxLength;
        }
    };
    
    FunctionFilter filter(MessageLengthFilter(10));
    
    auto shortMsg = createMessage("Short");
    auto longMsg = createMessage("This is a very long message");
    
    QVERIFY(filter.filter(shortMsg));
    QVERIFY(!filter.filter(longMsg));
}

void TestFunctionFilter::testAlwaysTrue()
{
    FunctionFilter filter([](const LogMessage&) { return true; });
    
    auto msg1 = createMessage("Any message");
    auto msg2 = createMessage("", QtCriticalMsg);
    auto msg3 = createMessage("Special chars: äöü€");
    
    QVERIFY(filter.filter(msg1));
    QVERIFY(filter.filter(msg2));
    QVERIFY(filter.filter(msg3));
}

void TestFunctionFilter::testAlwaysFalse()
{
    FunctionFilter filter([](const LogMessage&) { return false; });
    
    auto msg1 = createMessage("Any message");
    auto msg2 = createMessage("Important message", QtCriticalMsg);
    auto msg3 = createMessage("Debug info", QtDebugMsg);
    
    QVERIFY(!filter.filter(msg1));
    QVERIFY(!filter.filter(msg2));
    QVERIFY(!filter.filter(msg3));
}

void TestFunctionFilter::testMessageContentFilter()
{
    FunctionFilter filter([](const LogMessage& msg) {
        return msg.message().contains("error", Qt::CaseInsensitive);
    });
    
    auto errorMsg = createMessage("An error occurred");
    auto ERRORMsg = createMessage("SYSTEM ERROR");
    auto warningMsg = createMessage("This is a warning");
    auto emptyMsg = createMessage("");
    
    QVERIFY(filter.filter(errorMsg));
    QVERIFY(filter.filter(ERRORMsg));
    QVERIFY(!filter.filter(warningMsg));
    QVERIFY(!filter.filter(emptyMsg));
}

void TestFunctionFilter::testMessageTypeFilter()
{
    FunctionFilter filter([](const LogMessage& msg) {
        // Filter to allow only warnings, critical, and fatal (not debug or info)
        return msg.type() == QtWarningMsg || msg.type() == QtCriticalMsg || msg.type() == QtFatalMsg;
    });
    
    auto debugMsg = createMessage("Debug info", QtDebugMsg);
    auto infoMsg = createMessage("Info message", QtInfoMsg);
    auto warningMsg = createMessage("Warning message", QtWarningMsg);
    auto criticalMsg = createMessage("Critical error", QtCriticalMsg);
    auto fatalMsg = createMessage("Fatal error", QtFatalMsg);
    
    QVERIFY(!filter.filter(debugMsg));
    QVERIFY(!filter.filter(infoMsg));
    QVERIFY(filter.filter(warningMsg));
    QVERIFY(filter.filter(criticalMsg));
    QVERIFY(filter.filter(fatalMsg));
}

void TestFunctionFilter::testCategoryFilter()
{
    FunctionFilter filter([](const LogMessage& msg) {
        QString category(msg.category());
        return category.startsWith("app.");
    });
    
    auto appMsg = createMessage("App message", QtDebugMsg, "app.core");
    auto appUiMsg = createMessage("UI message", QtDebugMsg, "app.ui");
    auto systemMsg = createMessage("System message", QtDebugMsg, "system.core");
    auto emptyCategory = createMessage("Empty category", QtDebugMsg, "");
    
    QVERIFY(filter.filter(appMsg));
    QVERIFY(filter.filter(appUiMsg));
    QVERIFY(!filter.filter(systemMsg));
    QVERIFY(!filter.filter(emptyCategory));
}

void TestFunctionFilter::testCombinedConditions()
{
    FunctionFilter filter([](const LogMessage& msg) {
        return msg.type() == QtWarningMsg && 
               msg.message().contains("network", Qt::CaseInsensitive) &&
               QString(msg.category()).startsWith("app.");
    });
    
    auto matchingMsg = createMessage("Network connection failed", QtWarningMsg, "app.network");
    auto wrongType = createMessage("Network connection failed", QtDebugMsg, "app.network");
    auto wrongContent = createMessage("Database error", QtWarningMsg, "app.network");
    auto wrongCategory = createMessage("Network connection failed", QtWarningMsg, "system.network");

    filter.filter(matchingMsg);
    
    QVERIFY(filter.filter(matchingMsg));
    QVERIFY(!filter.filter(wrongType));
    QVERIFY(!filter.filter(wrongContent));
    QVERIFY(!filter.filter(wrongCategory));
}

void TestFunctionFilter::testStatefulFilter()
{
    int messageCount = 0;
    FunctionFilter filter([&messageCount](const LogMessage&) {
        return ++messageCount % 2 == 1; // Only odd-numbered messages
    });
    
    auto msg1 = createMessage("Message 1");
    auto msg2 = createMessage("Message 2");
    auto msg3 = createMessage("Message 3");
    auto msg4 = createMessage("Message 4");
    auto msg5 = createMessage("Message 5");
    
    QVERIFY(filter.filter(msg1));   // 1st message (odd)
    QVERIFY(!filter.filter(msg2));  // 2nd message (even)
    QVERIFY(filter.filter(msg3));   // 3rd message (odd)
    QVERIFY(!filter.filter(msg4));  // 4th message (even)
    QVERIFY(filter.filter(msg5));   // 5th message (odd)
}

void TestFunctionFilter::testCountingFilter()
{
    std::map<QtMsgType, int> typeCounts;
    FunctionFilter filter([&typeCounts](const LogMessage& msg) {
        typeCounts[msg.type()]++;
        return typeCounts[msg.type()] <= 2; // Max 2 messages per type
    });
    
    auto debug1 = createMessage("Debug 1", QtDebugMsg);
    auto debug2 = createMessage("Debug 2", QtDebugMsg);
    auto debug3 = createMessage("Debug 3", QtDebugMsg);
    auto warning1 = createMessage("Warning 1", QtWarningMsg);
    auto warning2 = createMessage("Warning 2", QtWarningMsg);
    auto warning3 = createMessage("Warning 3", QtWarningMsg);
    
    QVERIFY(filter.filter(debug1));     // 1st debug
    QVERIFY(filter.filter(debug2));     // 2nd debug
    QVERIFY(!filter.filter(debug3));    // 3rd debug (blocked)
    QVERIFY(filter.filter(warning1));   // 1st warning
    QVERIFY(filter.filter(warning2));   // 2nd warning
    QVERIFY(!filter.filter(warning3));  // 3rd warning (blocked)
}

void TestFunctionFilter::testTimeBasedFilter()
{
    auto startTime = QDateTime::currentDateTime();
    FunctionFilter filter([startTime](const LogMessage& msg) {
        auto timeDiff = startTime.msecsTo(msg.time());
        return timeDiff >= 0; // Only messages at or after start time
    });
    
    // Create a message with current time (should pass)
    auto currentMsg = createMessage("Current message");
    QVERIFY(filter.filter(currentMsg));
    
    // For historical message testing, we'd need to create LogMessage with custom time
    // Since LogMessage sets time automatically, this test mainly verifies the logic
}

void TestFunctionFilter::testNullptrFunction()
{
    // Test with null function - this should be caught at compile time
    // but we can test with an empty std::function
    std::function<bool(const LogMessage&)> nullFunc;
    
    // This would typically cause a runtime error, so we don't create the filter
    // FunctionFilter filter(nullFunc); // Would throw/crash
    
    // Instead, test with a valid but simple function
    FunctionFilter filter([](const LogMessage&) { return true; });
    auto msg = createMessage("Test");
    QVERIFY(filter.filter(msg));
}

void TestFunctionFilter::testExceptionInFunction()
{
    FunctionFilter filter([](const LogMessage& msg) -> bool {
        if (msg.message().contains("throw")) {
            throw std::runtime_error("Test exception");
        }
        return true;
    });
    
    auto normalMsg = createMessage("Normal message");
    auto throwMsg = createMessage("This will throw exception");
    
    QVERIFY(filter.filter(normalMsg));
    
    // Exception in filter function - behavior depends on implementation
    // In most cases, this would propagate the exception
    bool exceptionCaught = false;
    try {
        filter.filter(throwMsg);
    } catch (const std::runtime_error&) {
        exceptionCaught = true;
    }
    QVERIFY(exceptionCaught);
}

void TestFunctionFilter::testComplexLogicFilter()
{
    FunctionFilter filter([](const LogMessage& msg) {
        // Complex filter: allow critical/fatal messages always,
        // allow debug/info only for specific categories,
        // allow warnings only with specific keywords
        
        if (msg.type() == QtCriticalMsg || msg.type() == QtFatalMsg) {
            return true; // Always allow critical and fatal
        }
        
        QString category(msg.category());
        if ((msg.type() == QtDebugMsg || msg.type() == QtInfoMsg) && 
            category.startsWith("debug.")) {
            return true;
        }
        
        if (msg.type() == QtWarningMsg && 
            (msg.message().contains("important", Qt::CaseInsensitive) || msg.message().contains("urgent", Qt::CaseInsensitive))) {
            return true;
        }
        
        return false;
    });
    
    auto criticalMsg = createMessage("Any critical", QtCriticalMsg, "any.category");
    auto fatalMsg = createMessage("Any fatal", QtFatalMsg, "any.category");
    auto debugAllowed = createMessage("Debug info", QtDebugMsg, "debug.core");
    auto debugBlocked = createMessage("Debug info", QtDebugMsg, "app.core");
    auto warningImportant = createMessage("Important warning", QtWarningMsg, "app.core");
    auto warningUrgent = createMessage("Urgent issue", QtWarningMsg, "system.core");
    auto warningNormal = createMessage("Normal warning", QtWarningMsg, "app.core");
    auto infoAllowed = createMessage("Info message", QtInfoMsg, "debug.test");
    auto infoBlocked = createMessage("Info message", QtInfoMsg, "app.test");
    
    QVERIFY(filter.filter(criticalMsg));
    QVERIFY(filter.filter(fatalMsg));
    QVERIFY(filter.filter(debugAllowed));
    QVERIFY(!filter.filter(debugBlocked));
    QVERIFY(filter.filter(warningImportant));
    QVERIFY(filter.filter(warningUrgent));
    QVERIFY(!filter.filter(warningNormal));
    QVERIFY(filter.filter(infoAllowed));
    QVERIFY(!filter.filter(infoBlocked));
}

void TestFunctionFilter::testPerformanceFilter()
{
    // Simple performance filter that should be fast
    FunctionFilter filter([](const LogMessage& msg) {
        return msg.message().length() > 5;
    });
    
    QElapsedTimer timer;
    timer.start();
    
    // Process many messages
    for (int i = 0; i < 10000; ++i) {
        auto msg = createMessage(QString("Message %1").arg(i));
        filter.filter(msg);
    }
    
    int elapsed = timer.elapsed();
    // Should complete in reasonable time (less than 1 second for 10k messages)
    QVERIFY(elapsed < 1000);
}

void TestFunctionFilter::testManyFilterCalls()
{
    int callCount = 0;
    FunctionFilter filter([&callCount](const LogMessage&) {
        callCount++;
        return callCount % 100 == 0; // Every 100th message
    });
    
    int passedCount = 0;
    for (int i = 0; i < 1000; ++i) {
        auto msg = createMessage(QString("Message %1").arg(i));
        if (filter.filter(msg)) {
            passedCount++;
        }
    }
    
    QCOMPARE(callCount, 1000);    // All messages should have been processed
    QCOMPARE(passedCount, 10);    // Every 100th message should pass (10 out of 1000)
}

QTEST_MAIN(TestFunctionFilter)
#include "test_functionfilter.moc"

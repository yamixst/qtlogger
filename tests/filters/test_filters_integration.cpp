#include <QtTest/QtTest>
#include <QMessageLogContext>
#include <QSharedPointer>
#include <QElapsedTimer>

#include "qtlogger/filters/categoryfilter.h"
#include "qtlogger/filters/duplicatefilter.h"
#include "qtlogger/filters/functionfilter.h"
#include "qtlogger/filters/regexpfilter.h"
#include "qtlogger/handler.h"
#include "mock_context.h"

using namespace QtLogger;

class TestFiltersIntegration : public QObject
{
    Q_OBJECT

private slots:
    // Filter combination tests
    void testCategoryAndDuplicateFilters();
    void testRegExpAndFunctionFilters();
    void testAllFiltersChained();
    void testFilterOrderMatters();

    // Filter compatibility tests
    void testFilterTypeCasting();
    void testSharedPointerUsage();
    void testFilterFactoryPattern();

    // Real-world scenarios
    void testLogProcessingPipeline();
    void testPerformanceWithMultipleFilters();
    void testComplexFilteringScenario();

    // Edge cases and robustness
    void testEmptyFilterChain();
    void testNullFilterHandling();
    void testFilterExceptionHandling();

private:
    // Helper methods
    LogMessage createMessage(const QString& message, QtMsgType type = QtDebugMsg, 
                           const QString& category = "test.category");
    
    template<typename... Filters>
    bool processMessageThroughFilters(const LogMessage& msg, Filters... filters);
};

LogMessage TestFiltersIntegration::createMessage(const QString& message, QtMsgType type, const QString& category)
{
    auto context = Test::MockContext::createWithCategory(category);
    return LogMessage(type, context, message);
}

template<typename... Filters>
bool TestFiltersIntegration::processMessageThroughFilters(const LogMessage& msg, Filters... filters)
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

void TestFiltersIntegration::testCategoryAndDuplicateFilters()
{
    // Create filters
    CategoryFilter categoryFilter("app.*=true\nsystem.*=false");
    DuplicateFilter duplicateFilter;
    
    // Test messages
    auto appMsg1 = createMessage("App message 1", QtDebugMsg, "app.core");
    auto appMsg2 = createMessage("App message 1", QtDebugMsg, "app.core"); // Duplicate
    auto systemMsg = createMessage("System message", QtDebugMsg, "system.core");
    auto appMsg3 = createMessage("App message 2", QtDebugMsg, "app.ui");
    
    // Test category filter first, then duplicate filter
    QVERIFY(categoryFilter.filter(appMsg1));
    QVERIFY(duplicateFilter.filter(appMsg1));
    
    QVERIFY(categoryFilter.filter(appMsg2));  // Category allows it
    QVERIFY(!duplicateFilter.filter(appMsg2)); // Duplicate filter blocks it
    
    QVERIFY(!categoryFilter.filter(systemMsg)); // Category filter blocks it
    // Don't test duplicate filter since category already blocked it
    
    QVERIFY(categoryFilter.filter(appMsg3));
    QVERIFY(duplicateFilter.filter(appMsg3)); // Different message, should pass
}

void TestFiltersIntegration::testRegExpAndFunctionFilters()
{
    // RegExp filter for error messages
    RegExpFilter regexpFilter("error|warning|critical");
    
    // Function filter for message length
    FunctionFilter functionFilter([](const LogMessage& msg) {
        return msg.message().length() <= 50;
    });
    
    auto shortError = createMessage("Error");
    auto longError = createMessage("This is a very long error message that exceeds the length limit");
    auto shortInfo = createMessage("Info message");
    auto longInfo = createMessage("This is a very long info message that exceeds fifty characters");
    
    // Test RegExp filter first, then function filter
    QVERIFY(regexpFilter.filter(shortError));
    QVERIFY(functionFilter.filter(shortError));
    
    QVERIFY(regexpFilter.filter(longError));
    QVERIFY(!functionFilter.filter(longError));
    
    QVERIFY(!regexpFilter.filter(shortInfo));
    // Don't test function filter since regexp already blocked it
    
    QVERIFY(!regexpFilter.filter(longInfo));
}

void TestFiltersIntegration::testAllFiltersChained()
{
    // Create all filter types
    CategoryFilter categoryFilter("app.*=true");
    DuplicateFilter duplicateFilter;
    RegExpFilter regexpFilter("important");
    FunctionFilter functionFilter([](const LogMessage& msg) {
        return msg.type() >= QtWarningMsg;
    });
    
    auto validMsg = createMessage("Important warning", QtWarningMsg, "app.core");
    auto duplicateMsg = createMessage("Important warning", QtWarningMsg, "app.core");
    auto wrongCategory = createMessage("Important warning", QtWarningMsg, "system.core");
    auto wrongContent = createMessage("Normal warning", QtWarningMsg, "app.core");
    auto wrongType = createMessage("Important info", QtDebugMsg, "app.core");
    
    // Test valid message through all filters
    QVERIFY(categoryFilter.filter(validMsg));
    QVERIFY(duplicateFilter.filter(validMsg));
    QVERIFY(regexpFilter.filter(validMsg));
    QVERIFY(functionFilter.filter(validMsg));
    
    // Test duplicate message
    QVERIFY(categoryFilter.filter(duplicateMsg));
    QVERIFY(!duplicateFilter.filter(duplicateMsg)); // Should be blocked here
    
    // Test wrong category
    QVERIFY(!categoryFilter.filter(wrongCategory)); // Should be blocked here
    
    // Test wrong content
    QVERIFY(categoryFilter.filter(wrongContent));
    QVERIFY(duplicateFilter.filter(wrongContent));
    QVERIFY(!regexpFilter.filter(wrongContent)); // Should be blocked here
    
    // Test wrong type
    QVERIFY(categoryFilter.filter(wrongType));
    QVERIFY(duplicateFilter.filter(wrongType));
    QVERIFY(regexpFilter.filter(wrongType));
    QVERIFY(!functionFilter.filter(wrongType)); // Should be blocked here
}

void TestFiltersIntegration::testFilterOrderMatters()
{
    // Create filters that might have different behavior based on order
    DuplicateFilter duplicateFilter1, duplicateFilter2;
    CategoryFilter categoryFilter("test.*=false");
    
    auto msg1 = createMessage("Test message", QtDebugMsg, "test.category");
    auto msg2 = createMessage("Test message", QtDebugMsg, "test.category"); // Duplicate
    
    // Scenario 1: Duplicate filter first, then category filter
    QVERIFY(duplicateFilter1.filter(msg1));    // First message passes
    QVERIFY(!categoryFilter.filter(msg1));     // Category blocks it
    
    QVERIFY(!duplicateFilter1.filter(msg2));   // Duplicate blocked
    // Category filter doesn't get the duplicate since it was already blocked
    
    // Scenario 2: Category filter first, then duplicate filter
    QVERIFY(!categoryFilter.filter(msg1));     // Category blocks it immediately
    // Duplicate filter doesn't see the message since category blocked it
    
    auto msg3 = createMessage("Valid message", QtDebugMsg, "valid.category");
    auto msg4 = createMessage("Valid message", QtDebugMsg, "valid.category"); // Duplicate
    
    QVERIFY(categoryFilter.filter(msg3));      // Category allows it
    QVERIFY(duplicateFilter2.filter(msg3));    // First message passes
    
    QVERIFY(categoryFilter.filter(msg4));      // Category allows it
    QVERIFY(!duplicateFilter2.filter(msg4));   // Duplicate blocked
}

void TestFiltersIntegration::testFilterTypeCasting()
{
    // Test that all filters can be used as base Filter type
    CategoryFilterPtr categoryFilter = QSharedPointer<CategoryFilter>::create("app.*=true");
    DuplicateFilterPtr duplicateFilter = QSharedPointer<DuplicateFilter>::create();
    RegExpFilterPtr regexpFilter = QSharedPointer<RegExpFilter>::create("test");
    FunctionFilterPtr functionFilter = QSharedPointer<FunctionFilter>::create(
        [](const LogMessage&) { return true; });
    
    // Test polymorphic behavior
    QList<FilterPtr> filters = { categoryFilter, duplicateFilter, regexpFilter, functionFilter };
    
    auto msg = createMessage("test message", QtDebugMsg, "app.core");
    
    for (auto& filter : filters) {
        QVERIFY(filter->type() == Handler::HandlerType::Filter);
        
        // Test process method (inherited from Handler)
        bool processResult = filter->process(const_cast<LogMessage&>(msg));
        bool filterResult = filter->filter(msg);
        QCOMPARE(processResult, filterResult);
    }
}

void TestFiltersIntegration::testSharedPointerUsage()
{
    // Test shared pointer lifecycle and copying
    auto categoryFilter = QSharedPointer<CategoryFilter>::create("test.*=true");
    auto duplicateFilter = QSharedPointer<DuplicateFilter>::create();
    
    // Create copies
    auto categoryFilterCopy = categoryFilter;
    auto duplicateFilterCopy = duplicateFilter;
    
    auto msg = createMessage("test message", QtDebugMsg, "test.category");
    
    // Test that copies work correctly
    QVERIFY(categoryFilter->filter(msg) == categoryFilterCopy->filter(msg));
    
    // Test duplicate filter state sharing
    QVERIFY(duplicateFilter->filter(msg));
    QVERIFY(!duplicateFilterCopy->filter(msg)); // Should be blocked due to shared state
}

void TestFiltersIntegration::testFilterFactoryPattern()
{
    // Mock factory function for creating filters
    auto createFilter = [](const QString& type, const QString& config) -> FilterPtr {
        if (type == "category") {
            return QSharedPointer<CategoryFilter>::create(config);
        } else if (type == "duplicate") {
            return QSharedPointer<DuplicateFilter>::create();
        } else if (type == "regexp") {
            return QSharedPointer<RegExpFilter>::create(config);
        } else if (type == "function") {
            // Simple function filter that checks for "important" keyword
            auto func = [](const LogMessage& msg) {
                return msg.message().contains("important", Qt::CaseInsensitive);
            };
            return QSharedPointer<FunctionFilter>::create(func);
        }
        return nullptr;
    };
    
    // Create filters using factory
    auto categoryFilter = createFilter("category", "app.*=true");
    auto duplicateFilter = createFilter("duplicate", "");
    auto regexpFilter = createFilter("regexp", "error|warning");
    auto functionFilter = createFilter("function", "");
    
    QVERIFY(categoryFilter != nullptr);
    QVERIFY(duplicateFilter != nullptr);
    QVERIFY(regexpFilter != nullptr);
    QVERIFY(functionFilter != nullptr);
    
    // Test created filters
    auto testMsg = createMessage("Important error message", QtWarningMsg, "app.core");
    
    QVERIFY(categoryFilter->filter(testMsg));
    QVERIFY(duplicateFilter->filter(testMsg));
    QVERIFY(regexpFilter->filter(testMsg));
    QVERIFY(functionFilter->filter(testMsg));
}

void TestFiltersIntegration::testLogProcessingPipeline()
{
    // Simulate a real log processing pipeline
    struct LogPipeline {
        QList<FilterPtr> filters;
        
        void addFilter(FilterPtr filter) {
            filters.append(filter);
        }
        
        bool processMessage(const LogMessage& msg) {
            for (auto& filter : filters) {
                if (!filter->filter(msg)) {
                    return false; // Message blocked
                }
            }
            return true; // Message passed all filters
        }
    };
    
    LogPipeline pipeline;
    
    // Add filters in order
    pipeline.addFilter(QSharedPointer<CategoryFilter>::create("app.*=true\nsystem.*=false"));
    pipeline.addFilter(QSharedPointer<DuplicateFilter>::create());
    pipeline.addFilter(QSharedPointer<RegExpFilter>::create("(error|warning|info)"));
    pipeline.addFilter(QSharedPointer<FunctionFilter>::create([](const LogMessage& msg) {
        return msg.message().length() <= 100; // Limit message length
    }));
    
    // Test various messages
    auto validMsg = createMessage("App error occurred", QtWarningMsg, "app.core");
    auto duplicateMsg = createMessage("App error occurred", QtWarningMsg, "app.core");
    auto blockedCategory = createMessage("System error", QtWarningMsg, "system.core");
    auto blockedContent = createMessage("App debug message", QtDebugMsg, "app.core");
    auto blockedLength = createMessage(QString("App error: ").append("x").repeated(200), QtWarningMsg, "app.core");
    
    QVERIFY(pipeline.processMessage(validMsg));      // Should pass all filters
    QVERIFY(!pipeline.processMessage(duplicateMsg)); // Blocked by duplicate filter
    QVERIFY(!pipeline.processMessage(blockedCategory)); // Blocked by category filter
    QVERIFY(!pipeline.processMessage(blockedContent));  // Blocked by regexp filter
    QVERIFY(!pipeline.processMessage(blockedLength));   // Blocked by function filter
}

void TestFiltersIntegration::testPerformanceWithMultipleFilters()
{
    // Create multiple filters
    auto categoryFilter = QSharedPointer<CategoryFilter>::create("app.*=true");
    auto duplicateFilter = QSharedPointer<DuplicateFilter>::create();
    auto regexpFilter = QSharedPointer<RegExpFilter>::create("message_\\d+");
    auto functionFilter = QSharedPointer<FunctionFilter>::create([](const LogMessage& msg) {
        return msg.message().length() % 2 == 0; // Even length messages
    });
    
    QList<FilterPtr> filters = { categoryFilter, duplicateFilter, regexpFilter, functionFilter };
    
    QElapsedTimer timer;
    timer.start();
    
    int processedCount = 0;
    int passedCount = 0;
    
    // Process many messages
    for (int i = 0; i < 10000; ++i) {
        QString content = QString("message_%1").arg(i);
        auto msg = createMessage(content, QtDebugMsg, "app.test");
        
        processedCount++;
        bool passed = true;
        
        for (auto& filter : filters) {
            if (!filter->filter(msg)) {
                passed = false;
                break;
            }
        }
        
        if (passed) {
            passedCount++;
        }
    }
    
    int elapsed = timer.elapsed();
    
    QCOMPARE(processedCount, 10000);
    QVERIFY(passedCount > 0); // Some messages should pass
    QVERIFY(passedCount < processedCount); // Not all messages should pass
    QVERIFY(elapsed < 5000); // Should complete within 5 seconds
}

void TestFiltersIntegration::testComplexFilteringScenario()
{
    // Scenario: Log aggregation system with complex filtering requirements
    
    // 1. Category filter: Only allow app.* and database.* categories
    auto categoryFilter = QSharedPointer<CategoryFilter>::create(
        "app.*=true\ndatabase.*=true\nsystem.*=false\nnetwork.*=false");
    
    // 2. Duplicate filter: Remove consecutive duplicates
    auto duplicateFilter = QSharedPointer<DuplicateFilter>::create();
    
    // 3. Severity filter: Only warnings and above during business hours (simulated)
    auto severityFilter = QSharedPointer<FunctionFilter>::create([](const LogMessage& msg) {
        // Simulate business hours check (simplified)
        bool isBusinessHours = (msg.time().time().hour() >= 9 && msg.time().time().hour() < 17);
        if (isBusinessHours) {
            return msg.type() >= QtWarningMsg;
        }
        return true; // Allow all messages outside business hours
    });
    
    // 4. Content filter: Block sensitive information
    auto contentFilter = QSharedPointer<RegExpFilter>::create(
        "(?!.*(password|secret|key|token)).*");
    
    // 5. Rate limiting filter: Max 3 messages per category type combination
    std::map<QString, int> rateLimits;
    auto rateLimitFilter = QSharedPointer<FunctionFilter>::create([&rateLimits](const LogMessage& msg) {
        QString key = QString("%1_%2").arg(msg.category()).arg(qtMsgTypeToString(msg.type()));
        rateLimits[key]++;
        return rateLimits[key] <= 3;
    });
    
    QList<FilterPtr> filters = { 
        categoryFilter, 
        duplicateFilter, 
        severityFilter, 
        contentFilter, 
        rateLimitFilter 
    };
    
    // Test messages
    QStringList testMessages = {
        "Database connection established",
        "Database connection established", // Duplicate
        "App started successfully",
        "User password updated", // Contains sensitive info
        "Network timeout occurred", // Wrong category
        "Database query failed",
        "App warning: low memory",
        "Database error: connection lost",
        "App critical: system failure",
        "Database info: backup completed", // Might be filtered by severity during business hours
        "Database warning: slow query",
        "Database warning: slow query", // Another slow query
        "Database warning: another slow query", // Third warning (might hit rate limit)
        "Database warning: fourth slow query" // Fourth warning (should hit rate limit)
    };
    
    QStringList categories = {
        "database.connection", "database.connection", "app.startup", "app.security",
        "network.timeout", "database.query", "app.memory", "database.connection",
        "app.system", "database.backup", "database.performance", "database.performance",
        "database.performance", "database.performance"
    };
    
    QtMsgType types[] = {
        QtInfoMsg, QtInfoMsg, QtInfoMsg, QtInfoMsg,
        QtWarningMsg, QtCriticalMsg, QtWarningMsg, QtCriticalMsg,
        QtCriticalMsg, QtInfoMsg, QtWarningMsg, QtWarningMsg,
        QtWarningMsg, QtWarningMsg
    };
    
    int passedCount = 0;
    for (int i = 0; i < testMessages.size(); ++i) {
        auto msg = createMessage(testMessages[i], types[i], categories[i]);
        
        bool passed = true;
        for (auto& filter : filters) {
            if (!filter->filter(msg)) {
                passed = false;
                break;
            }
        }
        
        if (passed) {
            passedCount++;
        }
    }
    
    // We expect some messages to be filtered out
    QVERIFY(passedCount > 0);
    QVERIFY(passedCount < testMessages.size());
}

void TestFiltersIntegration::testEmptyFilterChain()
{
    QList<FilterPtr> emptyFilters;
    
    auto msg = createMessage("Test message");
    
    // Empty filter chain should allow all messages (vacuous truth)
    bool passed = true;
    for (auto& filter : emptyFilters) {
        if (!filter->filter(msg)) {
            passed = false;
            break;
        }
    }
    
    QVERIFY(passed); // Should pass with empty filter chain
}

void TestFiltersIntegration::testNullFilterHandling()
{
    // Test handling of null filters in a chain
    QList<FilterPtr> filtersWithNull;
    filtersWithNull.append(QSharedPointer<CategoryFilter>::create("app.*=true"));
    filtersWithNull.append(FilterPtr()); // Null filter
    filtersWithNull.append(QSharedPointer<DuplicateFilter>::create());
    
    auto msg = createMessage("Test message", QtDebugMsg, "app.core");
    
    // Process with null check
    bool passed = true;
    for (auto& filter : filtersWithNull) {
        if (filter && !filter->filter(msg)) {
            passed = false;
            break;
        }
    }
    
    QVERIFY(passed); // Should handle null filters gracefully
}

void TestFiltersIntegration::testFilterExceptionHandling()
{
    // Create a function filter that might throw exceptions
    auto throwingFilter = QSharedPointer<FunctionFilter>::create([](const LogMessage& msg) -> bool {
        if (msg.message().contains("crash")) {
            throw std::runtime_error("Simulated filter crash");
        }
        return true;
    });
    
    auto normalFilter = QSharedPointer<CategoryFilter>::create("app.*=true");
    
    QList<FilterPtr> filters = { normalFilter, throwingFilter };
    
    auto normalMsg = createMessage("Normal message", QtDebugMsg, "app.core");
    auto crashMsg = createMessage("This will crash filter", QtDebugMsg, "app.core");
    
    // Normal message should work
    bool normalPassed = true;
    for (auto& filter : filters) {
        if (!filter->filter(normalMsg)) {
            normalPassed = false;
            break;
        }
    }
    QVERIFY(normalPassed);
    
    // Crashing message should throw exception
    bool exceptionCaught = false;
    try {
        for (auto& filter : filters) {
            filter->filter(crashMsg);
        }
    } catch (const std::runtime_error&) {
        exceptionCaught = true;
    }
    QVERIFY(exceptionCaught);
}

QTEST_MAIN(TestFiltersIntegration)
#include "test_filters_integration.moc"
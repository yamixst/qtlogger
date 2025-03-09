#include <QtTest/QtTest>
#include <QMessageLogContext>

#include "qtlogger/filters/duplicatefilter.h"
#include "mock_context.h"

using namespace QtLogger;

class TestDuplicateFilter : public QObject
{
    Q_OBJECT

private slots:
    // Basic functionality tests
    void testFirstMessagePasses();
    void testDuplicateMessageFiltered();
    void testDifferentMessagesPass();
    void testAlternatingMessages();

    // Edge cases
    void testEmptyMessages();
    void testNullMessages();
    void testVeryLongMessages();
    void testSpecialCharacters();

    // State management tests
    void testFilterState();
    void testMultipleFilters();
    void testSequentialDuplicates();

    // Performance and stress tests
    void testManyDuplicates();
    void testManyUniqueMessages();

    // Integration with different message types
    void testDifferentLogTypes();
    void testSameCategoryDifferentContent();

private:
    LogMessage createMessage(const QString& message, QtMsgType type = QtDebugMsg, const QString& category = "test.category");
};

LogMessage TestDuplicateFilter::createMessage(const QString& message, QtMsgType type, const QString& category)
{
    auto context = Test::MockContext::createWithCategory(category);
    return LogMessage(type, context, message);
}

void TestDuplicateFilter::testFirstMessagePasses()
{
    DuplicateFilter filter;
    
    auto msg = createMessage("First message");
    QVERIFY(filter.filter(msg));
}

void TestDuplicateFilter::testDuplicateMessageFiltered()
{
    DuplicateFilter filter;
    
    auto msg1 = createMessage("Same message");
    auto msg2 = createMessage("Same message");
    
    QVERIFY(filter.filter(msg1));  // First message should pass
    QVERIFY(!filter.filter(msg2)); // Duplicate should be filtered
}

void TestDuplicateFilter::testDifferentMessagesPass()
{
    DuplicateFilter filter;
    
    auto msg1 = createMessage("First message");
    auto msg2 = createMessage("Second message");
    auto msg3 = createMessage("Third message");
    
    QVERIFY(filter.filter(msg1));
    QVERIFY(filter.filter(msg2));
    QVERIFY(filter.filter(msg3));
}

void TestDuplicateFilter::testAlternatingMessages()
{
    DuplicateFilter filter;
    
    auto msgA1 = createMessage("Message A");
    auto msgB1 = createMessage("Message B");
    auto msgA2 = createMessage("Message A");  // Same as first
    auto msgB2 = createMessage("Message B");  // Same as second
    
    QVERIFY(filter.filter(msgA1));   // First A passes
    QVERIFY(filter.filter(msgB1));   // First B passes (different from A)
    QVERIFY(filter.filter(msgA2));   // A passes again (last was B)
    QVERIFY(filter.filter(msgB2));   // B passes (last was A, so this B is different from last message)
}

void TestDuplicateFilter::testEmptyMessages()
{
    DuplicateFilter filter;
    
    auto msg1 = createMessage("");
    auto msg2 = createMessage("");
    auto msg3 = createMessage("Non-empty");
    auto msg4 = createMessage("");
    
    QVERIFY(!filter.filter(msg1));  // First empty message filtered (matches initial empty state)
    QVERIFY(!filter.filter(msg2));  // Duplicate empty message filtered
    QVERIFY(filter.filter(msg3));   // Non-empty message passes
    QVERIFY(filter.filter(msg4));   // Empty message passes (last was non-empty)
}

void TestDuplicateFilter::testNullMessages()
{
    DuplicateFilter filter;
    
    auto msg1 = createMessage(QString());
    auto msg2 = createMessage(QString());
    
    QVERIFY(!filter.filter(msg1));  // First null message filtered (matches initial empty state)
    QVERIFY(!filter.filter(msg2));  // Duplicate null message filtered
}

void TestDuplicateFilter::testVeryLongMessages()
{
    DuplicateFilter filter;
    
    QString longMessage = QString("Very long message ").repeated(1000);
    
    auto msg1 = createMessage(longMessage);
    auto msg2 = createMessage(longMessage);
    auto msg3 = createMessage(longMessage + "different");
    
    QVERIFY(filter.filter(msg1));   // First long message passes
    QVERIFY(!filter.filter(msg2));  // Duplicate long message filtered
    QVERIFY(filter.filter(msg3));   // Modified long message passes
}

void TestDuplicateFilter::testSpecialCharacters()
{
    DuplicateFilter filter;
    
    QString specialMessage = "Special chars: \n\t\r\"'\\€äöüß中文🙂";
    
    auto msg1 = createMessage(specialMessage);
    auto msg2 = createMessage(specialMessage);
    auto msg3 = createMessage("Special chars: \n\t\r\"'\\€äöüß中文🙃"); // Different emoji
    
    QVERIFY(filter.filter(msg1));   // First special message passes
    QVERIFY(!filter.filter(msg2));  // Duplicate special message filtered
    QVERIFY(filter.filter(msg3));   // Similar but different message passes
}

void TestDuplicateFilter::testFilterState()
{
    DuplicateFilter filter;
    
    auto msg1 = createMessage("Test message");
    auto msg2 = createMessage("Different message");
    auto msg3 = createMessage("Test message");  // Same as first
    
    QVERIFY(filter.filter(msg1));   // First message passes
    QVERIFY(filter.filter(msg2));   // Different message passes
    QVERIFY(filter.filter(msg3));   // First message passes again (last was different)
}

void TestDuplicateFilter::testMultipleFilters()
{
    DuplicateFilter filter1;
    DuplicateFilter filter2;
    
    auto msg1 = createMessage("Shared message");
    auto msg2 = createMessage("Shared message");
    
    // Each filter should maintain its own state
    QVERIFY(filter1.filter(msg1));   // First filter: first message passes
    QVERIFY(filter2.filter(msg1));   // Second filter: first message passes
    QVERIFY(!filter1.filter(msg2));  // First filter: duplicate filtered
    QVERIFY(!filter2.filter(msg2));  // Second filter: duplicate filtered
}

void TestDuplicateFilter::testSequentialDuplicates()
{
    DuplicateFilter filter;
    
    auto msg1 = createMessage("Repeated message");
    auto msg2 = createMessage("Repeated message");
    auto msg3 = createMessage("Repeated message");
    auto msg4 = createMessage("Repeated message");
    auto msg5 = createMessage("Different message");
    auto msg6 = createMessage("Repeated message");
    
    QVERIFY(filter.filter(msg1));   // First passes
    QVERIFY(!filter.filter(msg2));  // Duplicate filtered
    QVERIFY(!filter.filter(msg3));  // Still duplicate filtered
    QVERIFY(!filter.filter(msg4));  // Still duplicate filtered
    QVERIFY(filter.filter(msg5));   // Different message passes
    QVERIFY(filter.filter(msg6));   // Original message passes again (last was different)
}

void TestDuplicateFilter::testManyDuplicates()
{
    DuplicateFilter filter;
    
    QString message = "Repeated message";
    auto firstMsg = createMessage(message);
    
    // First message should pass
    QVERIFY(filter.filter(firstMsg));
    
    // Many duplicates should all be filtered
    for (int i = 0; i < 1000; ++i) {
        auto duplicateMsg = createMessage(message);
        QVERIFY(!filter.filter(duplicateMsg));
    }
    
    // Different message should pass
    auto differentMsg = createMessage("Different message");
    QVERIFY(filter.filter(differentMsg));
}

void TestDuplicateFilter::testManyUniqueMessages()
{
    DuplicateFilter filter;
    
    // Many unique messages should all pass
    for (int i = 0; i < 1000; ++i) {
        auto uniqueMsg = createMessage(QString("Message %1").arg(i));
        QVERIFY(filter.filter(uniqueMsg));
    }
}

void TestDuplicateFilter::testDifferentLogTypes()
{
    DuplicateFilter filter;
    
    // Same message content but different log types should be treated as duplicates
    // since the filter only looks at message content, not type
    auto debugMsg = createMessage("Same content", QtDebugMsg);
    auto warningMsg = createMessage("Same content", QtWarningMsg);
    auto infoMsg = createMessage("Same content", QtInfoMsg);
    
    QVERIFY(filter.filter(debugMsg));     // First message passes
    QVERIFY(!filter.filter(warningMsg));  // Same content, should be filtered
    QVERIFY(!filter.filter(infoMsg));     // Same content, should be filtered
    
    // Different content should pass regardless of type
    auto differentWarning = createMessage("Different content", QtWarningMsg);
    QVERIFY(filter.filter(differentWarning));
}

void TestDuplicateFilter::testSameCategoryDifferentContent()
{
    DuplicateFilter filter;
    
    // Same category but different content should pass
    auto msg1 = createMessage("Message 1", QtDebugMsg, "app.core");
    auto msg2 = createMessage("Message 2", QtDebugMsg, "app.core");
    auto msg3 = createMessage("Message 1", QtDebugMsg, "app.core");  // Same as first
    
    QVERIFY(filter.filter(msg1));   // First message passes
    QVERIFY(filter.filter(msg2));   // Different content passes
    QVERIFY(filter.filter(msg3));   // Same as first, but last was different, so passes
}

QTEST_MAIN(TestDuplicateFilter)
#include "test_duplicatefilter.moc"
#include <QtTest/QtTest>
#include <QMessageLogContext>
#include <QRegularExpression>
#include <QElapsedTimer>

#include "qtlogger/filters/regexpfilter.h"
#include "mock_context.h"

using namespace QtLogger;

class TestRegExpFilter : public QObject
{
    Q_OBJECT

private slots:
    // Constructor tests
    void testConstructorWithQRegularExpression();
    void testConstructorWithString();
    void testConstructorWithInvalidRegExp();

    // Basic pattern matching tests
    void testSimplePattern();
    void testCaseSensitiveMatching();
    void testCaseInsensitiveMatching();
    void testEmptyPattern();
    void testEmptyMessage();

    // Advanced regex patterns tests
    void testWildcardPattern();
    void testCharacterClasses();
    void testQuantifiers();
    void testGroupsAndCaptures();
    void testAnchorsAndBoundaries();

    // Edge cases and robustness tests
    void testSpecialCharacters();
    void testUnicodeCharacters();
    void testVeryLongMessages();
    void testComplexPatterns();
    void testInvalidRegexPattern();

    // Performance tests
    void testManyMessages();
    void testComplexRegexPerformance();

    // Integration with different message types
    void testDifferentLogTypes();
    void testMultilineMessages();

private:
    LogMessage createMessage(const QString& message, QtMsgType type = QtDebugMsg, 
                           const QString& category = "test.category");
};

LogMessage TestRegExpFilter::createMessage(const QString& message, QtMsgType type, const QString& category)
{
    auto context = Test::MockContext::createWithCategory(category);
    return LogMessage(type, context, message);
}

void TestRegExpFilter::testConstructorWithQRegularExpression()
{
    QRegularExpression regex("test.*pattern");
    RegExpFilter filter(regex);
    
    auto matchingMsg = createMessage("test message pattern");
    auto nonMatchingMsg = createMessage("sample message");
    
    QVERIFY(filter.filter(matchingMsg));
    QVERIFY(!filter.filter(nonMatchingMsg));
}

void TestRegExpFilter::testConstructorWithString()
{
    QString pattern = "error|warning|critical";
    RegExpFilter filter(pattern);
    
    auto errorMsg = createMessage("An error occurred");
    auto warningMsg = createMessage("This is a warning");
    auto criticalMsg = createMessage("Critical system failure");
    auto infoMsg = createMessage("Information message");
    
    QVERIFY(filter.filter(errorMsg));
    QVERIFY(filter.filter(warningMsg));
    QVERIFY(filter.filter(criticalMsg));
    QVERIFY(!filter.filter(infoMsg));
}

void TestRegExpFilter::testConstructorWithInvalidRegExp()
{
    // Test with invalid regex pattern
    QString invalidPattern = "[unclosed bracket";
    RegExpFilter filter(invalidPattern);
    
    // The filter should handle invalid regex gracefully
    auto msg = createMessage("test message");
    
    // Invalid regex might not match anything or might throw
    // The exact behavior depends on QRegularExpression implementation
    bool result = filter.filter(msg);
    Q_UNUSED(result); // We just want to ensure it doesn't crash
    QVERIFY(true); // Test passes if we reach here without crashing
}

void TestRegExpFilter::testSimplePattern()
{
    RegExpFilter filter("hello");
    
    auto matchingMsg = createMessage("hello world");
    auto matchingMsg2 = createMessage("say hello to everyone");
    auto nonMatchingMsg = createMessage("hi there");
    auto caseMsg = createMessage("Hello World"); // Different case
    
    QVERIFY(filter.filter(matchingMsg));
    QVERIFY(filter.filter(matchingMsg2));
    QVERIFY(!filter.filter(nonMatchingMsg));
    QVERIFY(!filter.filter(caseMsg)); // Should be case sensitive by default
}

void TestRegExpFilter::testCaseSensitiveMatching()
{
    RegExpFilter filter("Error");
    
    auto exactMatch = createMessage("Error occurred");
    auto lowerCase = createMessage("error occurred");
    auto upperCase = createMessage("ERROR OCCURRED");
    
    QVERIFY(filter.filter(exactMatch));
    QVERIFY(!filter.filter(lowerCase));
    QVERIFY(!filter.filter(upperCase));
}

void TestRegExpFilter::testCaseInsensitiveMatching()
{
    QRegularExpression regex("error", QRegularExpression::CaseInsensitiveOption);
    RegExpFilter filter(regex);
    
    auto lowerCase = createMessage("error occurred");
    auto upperCase = createMessage("ERROR OCCURRED");
    auto mixedCase = createMessage("Error Occurred");
    auto nonMatching = createMessage("warning issued");
    
    QVERIFY(filter.filter(lowerCase));
    QVERIFY(filter.filter(upperCase));
    QVERIFY(filter.filter(mixedCase));
    QVERIFY(!filter.filter(nonMatching));
}

void TestRegExpFilter::testEmptyPattern()
{
    RegExpFilter filter("");
    
    auto msg = createMessage("Any message");
    auto emptyMsg = createMessage("");
    
    // Empty pattern should match at the beginning of any string
    QVERIFY(filter.filter(msg));
    QVERIFY(filter.filter(emptyMsg));
}

void TestRegExpFilter::testEmptyMessage()
{
    RegExpFilter filter("test");
    
    auto emptyMsg = createMessage("");
    
    QVERIFY(!filter.filter(emptyMsg));
}

void TestRegExpFilter::testWildcardPattern()
{
    RegExpFilter filter("file.*\\.txt");
    
    auto txtFile = createMessage("Opening file document.txt");
    auto logFile = createMessage("Opening file application.log");
    auto txtFile2 = createMessage("Closing file data.txt for writing");
    
    QVERIFY(filter.filter(txtFile));
    QVERIFY(!filter.filter(logFile));
    QVERIFY(filter.filter(txtFile2));
}

void TestRegExpFilter::testCharacterClasses()
{
    RegExpFilter filter("\\d{3}-\\d{3}-\\d{4}"); // Phone number pattern
    
    auto validPhone = createMessage("Call 555-123-4567 for support");
    auto invalidPhone = createMessage("Call 555-12-4567 for support");
    auto noPhone = createMessage("Contact support via email");
    
    QVERIFY(filter.filter(validPhone));
    QVERIFY(!filter.filter(invalidPhone));
    QVERIFY(!filter.filter(noPhone));
}

void TestRegExpFilter::testQuantifiers()
{
    RegExpFilter filter("a{2,4}b+c*"); // a appears 2-4 times, b one or more, c zero or more
    
    auto match1 = createMessage("aabbcc matches pattern");
    auto match2 = createMessage("aaaabbbbb matches pattern");
    auto match3 = createMessage("aaaab matches pattern");
    auto noMatch1 = createMessage("ab does not match");
    auto noMatch2 = createMessage("aaaaab does not match");
    
    QVERIFY(filter.filter(match1));
    QVERIFY(filter.filter(match2));
    QVERIFY(filter.filter(match3));
    QVERIFY(!filter.filter(noMatch1));
    QVERIFY(!filter.filter(noMatch2));
}

void TestRegExpFilter::testGroupsAndCaptures()
{
    RegExpFilter filter("(error|warning):\\s*(\\w+)");
    
    auto errorMsg = createMessage("error: database connection failed");
    auto warningMsg = createMessage("warning: low disk space");
    auto infoMsg = createMessage("info: system started");
    
    QVERIFY(filter.filter(errorMsg));
    QVERIFY(filter.filter(warningMsg));
    QVERIFY(!filter.filter(infoMsg));
}

void TestRegExpFilter::testAnchorsAndBoundaries()
{
    RegExpFilter filter("^ERROR:.*$");
    
    auto startWithError = createMessage("ERROR: System failure detected");
    auto containsError = createMessage("System ERROR: Failure detected");
    auto endWithError = createMessage("System failure: ERROR");
    
    QVERIFY(filter.filter(startWithError));
    QVERIFY(!filter.filter(containsError));
    QVERIFY(!filter.filter(endWithError));
}

void TestRegExpFilter::testSpecialCharacters()
{
    RegExpFilter filter("\\[\\d+\\]\\s*\\w+");
    
    auto bracketMsg = createMessage("[123] Processing request");
    auto noBracketMsg = createMessage("123 Processing request");
    auto wrongBracketMsg = createMessage("(123) Processing request");
    
    QVERIFY(filter.filter(bracketMsg));
    QVERIFY(!filter.filter(noBracketMsg));
    QVERIFY(!filter.filter(wrongBracketMsg));
}

void TestRegExpFilter::testUnicodeCharacters()
{
    RegExpFilter filter("測試.*訊息"); // Chinese characters
    
    auto unicodeMsg = createMessage("測試系統訊息");
    auto partialUnicode = createMessage("測試 system message");
    auto noUnicode = createMessage("test system message");
    
    QVERIFY(filter.filter(unicodeMsg));
    QVERIFY(!filter.filter(partialUnicode));
    QVERIFY(!filter.filter(noUnicode));
}

void TestRegExpFilter::testVeryLongMessages()
{
    RegExpFilter filter("needle");
    
    QString longMessage = QString("hay ").repeated(10000) + "needle" + QString(" hay").repeated(10000);
    QString longMessageNoMatch = QString("hay ").repeated(20000);
    
    auto longMsg = createMessage(longMessage);
    auto longMsgNoMatch = createMessage(longMessageNoMatch);
    
    QVERIFY(filter.filter(longMsg));
    QVERIFY(!filter.filter(longMsgNoMatch));
}

void TestRegExpFilter::testComplexPatterns()
{
    // Email pattern
    RegExpFilter filter("\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Z|a-z]{2,}\\b");
    
    auto validEmail = createMessage("Contact user@example.com for more info");
    auto invalidEmail = createMessage("Contact user@invalid for more info");
    auto noEmail = createMessage("Contact support department for help");
    
    QVERIFY(filter.filter(validEmail));
    QVERIFY(!filter.filter(invalidEmail));
    QVERIFY(!filter.filter(noEmail));
}

void TestRegExpFilter::testInvalidRegexPattern()
{
    // Test various invalid patterns
    QStringList invalidPatterns = {
        "[unclosed",
        "*invalid",
        "(?invalid)",
        "\\x", // incomplete escape
        "(?P<>invalid)" // invalid named group
    };
    
    for (const QString& pattern : invalidPatterns) {
        RegExpFilter filter(pattern);
        auto msg = createMessage("test message");
        
        // Should not crash, behavior may vary
        try {
            bool result = filter.filter(msg);
            Q_UNUSED(result);
        } catch (...) {
            // Some invalid patterns might throw, which is acceptable
        }
        
        QVERIFY(true); // Test passes if we don't crash
    }
}

void TestRegExpFilter::testManyMessages()
{
    RegExpFilter filter("message_\\d+");
    
    int matchCount = 0;
    for (int i = 0; i < 10000; ++i) {
        QString content = (i % 2 == 0) ? QString("message_%1").arg(i) : QString("text_%1").arg(i);
        auto msg = createMessage(content);
        if (filter.filter(msg)) {
            matchCount++;
        }
    }
    
    QCOMPARE(matchCount, 5000); // Half should match
}

void TestRegExpFilter::testComplexRegexPerformance()
{
    // Complex regex that might be slow
    RegExpFilter filter("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    
    QElapsedTimer timer;
    timer.start();
    
    QStringList testIPs = {
        "192.168.1.1",
        "10.0.0.1",
        "255.255.255.255",
        "invalid.ip.address",
        "192.168.1.256", // Invalid
        "300.1.1.1" // Invalid
    };
    
    for (int i = 0; i < 1000; ++i) {
        for (const QString& ip : testIPs) {
            auto msg = createMessage(QString("IP address: %1").arg(ip));
            filter.filter(msg);
        }
    }
    
    int elapsed = timer.elapsed();
    // Should complete in reasonable time
    QVERIFY(elapsed < 5000); // 5 seconds should be more than enough
}

void TestRegExpFilter::testDifferentLogTypes()
{
    RegExpFilter filter("connection");
    
    auto debugMsg = createMessage("Database connection established", QtDebugMsg);
    auto warningMsg = createMessage("Connection timeout warning", QtWarningMsg);
    auto errorMsg = createMessage("Connection failed", QtCriticalMsg);
    auto noMatchMsg = createMessage("System startup complete", QtInfoMsg);
    
    // Filter should work regardless of log type
    QVERIFY(filter.filter(debugMsg));
    QVERIFY(filter.filter(warningMsg));
    QVERIFY(filter.filter(errorMsg));
    QVERIFY(!filter.filter(noMatchMsg));
}

void TestRegExpFilter::testMultilineMessages()
{
    QRegularExpression regex("line1.*line2", QRegularExpression::DotMatchesEverythingOption);
    RegExpFilter filter(regex);
    
    auto multilineMsg = createMessage("line1\nsome content\nline2");
    auto singleLineMsg = createMessage("line1 and line2 on same line");
    auto noMatchMsg = createMessage("line1\nsome content\nline3");
    
    QVERIFY(filter.filter(multilineMsg));
    QVERIFY(filter.filter(singleLineMsg));
    QVERIFY(!filter.filter(noMatchMsg));
}

QTEST_MAIN(TestRegExpFilter)
#include "test_regexpfilter.moc"
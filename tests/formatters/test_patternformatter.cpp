#include <QtTest/QtTest>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "qtlogger.h"
#include "mock_logmessage.h"
#include "qtlogger/formatters/patternformatter.h"

using namespace QtLogger;
using namespace QtLogger::Test;

class TestPatternFormatter : public QObject
{
    Q_OBJECT

private slots:
    // Basic PatternFormatter tests
    void testPatternFormatterBasic();
    void testPatternFormatterCustomPattern();
    void testPatternFormatterMultipleMessages();
    
    // Edge case tests
    void testPatternFormatterWithEmptyMessage();
    void testPatternFormatterWithLongMessage();
    void testPatternFormatterWithSpecialCharacters();
    
    // Time formatting tests
    void testPatternFormatterWithDefaultTimeFormat();
    void testPatternFormatterWithCustomTimeFormat();
    void testPatternFormatterWithMultipleTimeFormats();
    void testPatternFormatterWithProcessTime();
    void testPatternFormatterWithBootTime();
    
    // Custom attributes tests
    void testPatternFormatterWithCustomAttributes();
    void testPatternFormatterWithMissingAttributes();
    void testPatternFormatterWithMixedAttributes();

    // Shortfile tests
    void testPatternFormatterWithShortfile();
    void testPatternFormatterWithShortfileBaseDir();

    // Thread tests
    void testPatternFormatterWithQThreadPtr();

    // Optional attribute tests
    void testPatternFormatterWithOptionalAttribute();
    void testPatternFormatterWithOptionalAttributeRemoveBefore();
    void testPatternFormatterWithOptionalAttributeRemoveAfter();
    void testPatternFormatterWithOptionalAttributeRemoveBeforeAndAfter();

    // Fixed-width formatting tests
    void testPatternFormatterWithLeftAlign();
    void testPatternFormatterWithRightAlign();
    void testPatternFormatterWithCenterAlign();
    void testPatternFormatterWithCustomFillChar();
    void testPatternFormatterWithWidthSmallerThanContent();
    void testPatternFormatterWithMultipleFormattedTokens();
    void testPatternFormatterWithTimeAndFormatSpec();

    // Truncation tests (! suffix)
    void testPatternFormatterWithTruncation();
    void testPatternFormatterWithTruncationAndAlign();
    void testPatternFormatterWithTruncationAndFillChar();
    void testPatternFormatterWithTruncationShorterContent();
};

void TestPatternFormatter::testPatternFormatterBasic()
{
    QString pattern = "%{time} [%{type}] %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg = MockLogMessage::create(QtInfoMsg, "Pattern test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Pattern test"));
    QVERIFY(formatted.contains("[info]") || formatted.contains("[Info]") || formatted.contains("info"));
}

void TestPatternFormatter::testPatternFormatterCustomPattern()
{
    QString pattern = "%{file}:%{line} - %{function}() - %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg = MockLogMessage::createWithLocation("custom.cpp", 789, QtWarningMsg, "Custom pattern test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Custom pattern test"));
    // Note: The exact format depends on Qt's qFormatLogMessage implementation
}

void TestPatternFormatter::testPatternFormatterMultipleMessages()
{
    QString pattern = "[%{type}] %{category}: %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg1 = MockLogMessage::createWithCategory("app.core", QtDebugMsg, "Debug message");
    auto msg2 = MockLogMessage::createWithCategory("app.ui", QtWarningMsg, "Warning message");
    
    QString formatted1 = formatter.format(msg1);
    QString formatted2 = formatter.format(msg2);
    
    QVERIFY(!formatted1.isEmpty());
    QVERIFY(!formatted2.isEmpty());
    QVERIFY(formatted1.contains("Debug message"));
    QVERIFY(formatted2.contains("Warning message"));
}

void TestPatternFormatter::testPatternFormatterWithEmptyMessage()
{
    auto emptyMsg = MockLogMessage::createEmpty();
    
    // PatternFormatter
    PatternFormatter patternFormatter("%{message}");
    QString patternFormatted = patternFormatter.format(emptyMsg);
    // PatternFormatter may return null QString for empty messages, but should not crash
    Q_UNUSED(patternFormatted);
}

void TestPatternFormatter::testPatternFormatterWithLongMessage()
{
    auto longMsg = MockLogMessage::createLong();
    
    PatternFormatter patternFormatter("%{message}");
    QString patternFormatted = patternFormatter.format(longMsg);
    QVERIFY(!patternFormatted.isEmpty());
}

void TestPatternFormatter::testPatternFormatterWithSpecialCharacters()
{
    QString specialMessage = "Special chars: \n\t\r\"'\\€äöüß中文🙂";
    auto specialMsg = MockLogMessage::create(QtDebugMsg, specialMessage);
    
    PatternFormatter patternFormatter("%{message}");
    QString patternFormatted = patternFormatter.format(specialMsg);
    QVERIFY(patternFormatted.contains("Special chars"));
}

void TestPatternFormatter::testPatternFormatterWithCustomAttributes()
{
    QString pattern = "%{time} [%{type}] User: %{user} - ReqID: %{requestId} - %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg = MockLogMessage::create(QtInfoMsg, "Request completed");
    msg.setAttribute("user", "john.doe");
    msg.setAttribute("requestId", "12345");
    
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Request completed"));
    QVERIFY(formatted.contains("User: john.doe"));
    QVERIFY(formatted.contains("ReqID: 12345"));
}

void TestPatternFormatter::testPatternFormatterWithMissingAttributes()
{
    QString pattern = "User: %{user} - Duration: %{duration}ms - %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg = MockLogMessage::create(QtInfoMsg, "Test message");
    msg.setAttribute("user", "jane.smith");
    // Note: "duration" attribute is NOT set
    
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Test message"));
    QVERIFY(formatted.contains("User: jane.smith"));
    QVERIFY(formatted.contains("%{duration}"));  // Missing attribute should appear as literal
}

void TestPatternFormatter::testPatternFormatterWithMixedAttributes()
{
    QString pattern = "%{type} | %{category} | %{customAttr} | %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg = MockLogMessage::createWithCategory("app.test", QtWarningMsg, "Mixed test");
    msg.setAttribute("customAttr", "customValue");
    
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Mixed test"));
    QVERIFY(formatted.contains("app.test"));
    QVERIFY(formatted.contains("customValue"));
}

void TestPatternFormatter::testPatternFormatterWithDefaultTimeFormat()
{
    QString pattern = "%{time} - %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg = MockLogMessage::create(QtInfoMsg, "Time test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Time test"));
    // Default format should be ISO date format (contains 'T' separator and dashes)
    QVERIFY(formatted.contains("T") || formatted.contains("-"));
}

void TestPatternFormatter::testPatternFormatterWithCustomTimeFormat()
{
    QString pattern = "%{time yyyy-MM-dd} | %{time hh:mm:ss} | %{message}";
    PatternFormatter formatter(pattern);
    
    QDateTime testTime = QDateTime::currentDateTime();
    auto msg = MockLogMessage::create(QtInfoMsg, "Custom time format");
    
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Custom time format"));
    
    // Check that the format contains date separators (dashes) and time separators (colons)
    QVERIFY(formatted.contains("-"));
    QVERIFY(formatted.contains(":"));
    
    // The formatted string should contain two pipe separators from the pattern
    QCOMPARE(formatted.count("|"), 2);
}

void TestPatternFormatter::testPatternFormatterWithMultipleTimeFormats()
{
    QString pattern = "Date: %{time dd.MM.yyyy}, Time: %{time HH:mm:ss.zzz}, ISO: %{time} - %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg = MockLogMessage::create(QtDebugMsg, "Multiple formats");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Multiple formats"));
    QVERIFY(formatted.contains("Date:"));
    QVERIFY(formatted.contains("Time:"));
    QVERIFY(formatted.contains("ISO:"));
    
    // Check for dot separators in date (dd.MM.yyyy format)
    QVERIFY(formatted.contains("."));
    
    // Check for colon separators in time
    QVERIFY(formatted.contains(":"));
}

void TestPatternFormatter::testPatternFormatterWithProcessTime()
{
    QString pattern = "Process time: %{time process}s - %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg = MockLogMessage::create(QtInfoMsg, "Process time test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Process time test"));
    QVERIFY(formatted.contains("Process time:"));
    QVERIFY(formatted.contains("s - "));
    
    // Extract the time value and check it's a valid number
    QRegularExpression re("Process time: ([0-9.]+)s");
    QRegularExpressionMatch match = re.match(formatted);
    QVERIFY(match.hasMatch());
    
    QString timeStr = match.captured(1);
    bool ok = false;
    double timeValue = timeStr.toDouble(&ok);
    QVERIFY(ok);
    QVERIFY(timeValue >= 0.0);  // Time should be non-negative
}

void TestPatternFormatter::testPatternFormatterWithBootTime()
{
    QString pattern = "Boot time: %{time boot}s - %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg = MockLogMessage::create(QtDebugMsg, "Boot time test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Boot time test"));
    QVERIFY(formatted.contains("Boot time:"));
    QVERIFY(formatted.contains("s - "));
    
    // Extract the time value and check it's a valid number
    QRegularExpression re("Boot time: ([0-9.]+)s");
    QRegularExpressionMatch match = re.match(formatted);
    QVERIFY(match.hasMatch());
    
    QString timeStr = match.captured(1);
    bool ok = false;
    double timeValue = timeStr.toDouble(&ok);
    QVERIFY(ok);
    // Boot time could be any value depending on system, just verify it's a number
}




void TestPatternFormatter::testPatternFormatterWithShortfile()
{
    QString pattern = "%{shortfile}:%{line} - %{message}";
    PatternFormatter formatter(pattern);

    auto msg = MockLogMessage::createWithLocation("/home/user/project/src/module/file.cpp", 42, QtDebugMsg, "Shortfile test");
    QString formatted = formatter.format(msg);

    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Shortfile test"));
    // Without basedir, should only contain the filename
    QVERIFY(formatted.contains("file.cpp:42"));
    QVERIFY(!formatted.contains("/home/user"));
    QVERIFY(!formatted.contains("module/"));
}

void TestPatternFormatter::testPatternFormatterWithShortfileBaseDir()
{
    QString pattern = "%{shortfile /home/user/project}:%{line} - %{message}";
    PatternFormatter formatter(pattern);

    auto msg = MockLogMessage::createWithLocation("/home/user/project/src/module/file.cpp", 123, QtWarningMsg, "Basedir test");
    QString formatted = formatter.format(msg);

    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Basedir test"));
    // With basedir, should contain relative path from basedir
    QVERIFY(formatted.contains("src/module/file.cpp:123"));
    QVERIFY(!formatted.contains("/home/user/project"));

    // Test when file path doesn't start with basedir
    QString pattern2 = "%{shortfile /other/path}:%{line} - %{message}";
    PatternFormatter formatter2(pattern2);

    auto msg2 = MockLogMessage::createWithLocation("/home/user/project/src/file.cpp", 456, QtInfoMsg, "No match test");
    QString formatted2 = formatter2.format(msg2);

    // Should keep the full path if basedir doesn't match
    QVERIFY(formatted2.contains("/home/user/project/src/file.cpp:456"));
}

void TestPatternFormatter::testPatternFormatterWithQThreadPtr()
{
    QString pattern = "Thread: %{qthreadptr} - %{message}";
    PatternFormatter formatter(pattern);

    auto msg = MockLogMessage::create(QtDebugMsg, "Thread pointer test");
    QString formatted = formatter.format(msg);

    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Thread pointer test"));
    QVERIFY(formatted.contains("Thread: 0x"));

    // Extract the thread pointer and verify it's a valid hex number
    QRegularExpression re("Thread: (0x[0-9a-fA-F]+)");
    QRegularExpressionMatch match = re.match(formatted);
    QVERIFY(match.hasMatch());

    QString ptrStr = match.captured(1);
    QVERIFY(ptrStr.startsWith("0x"));
    
    bool ok = false;
    quintptr ptrValue = ptrStr.mid(2).toULongLong(&ok, 16);
    QVERIFY(ok);
    QVERIFY(ptrValue != 0);  // Thread pointer should not be null
}

void TestPatternFormatter::testPatternFormatterWithOptionalAttribute()
{
    // Test %{attr?} - if attribute not found, nothing is inserted
    QString pattern = "prefix[%{myattr?}]suffix";
    PatternFormatter formatter(pattern);

    // With attribute set
    auto msg1 = MockLogMessage::create(QtInfoMsg, "test");
    msg1.setAttribute("myattr", "VALUE");
    QString formatted1 = formatter.format(msg1);
    QCOMPARE(formatted1, QString("prefix[VALUE]suffix"));

    // Without attribute - should insert nothing for the attribute
    auto msg2 = MockLogMessage::create(QtInfoMsg, "test");
    QString formatted2 = formatter.format(msg2);
    QCOMPARE(formatted2, QString("prefix[]suffix"));
}

void TestPatternFormatter::testPatternFormatterWithOptionalAttributeRemoveBefore()
{
    // Test %{attr?N} - if attribute not found, remove N chars before
    QString pattern = "time //%{attrname?2} message";
    PatternFormatter formatter(pattern);

    // With attribute set
    auto msg1 = MockLogMessage::create(QtInfoMsg, "test");
    msg1.setAttribute("attrname", "VALUE");
    QString formatted1 = formatter.format(msg1);
    QCOMPARE(formatted1, QString("time //VALUE message"));

    // Without attribute - should remove 2 chars before ("//" becomes "")
    auto msg2 = MockLogMessage::create(QtInfoMsg, "test");
    QString formatted2 = formatter.format(msg2);
    QCOMPARE(formatted2, QString("time  message"));
}

void TestPatternFormatter::testPatternFormatterWithOptionalAttributeRemoveAfter()
{
    // Test %{attr?,M} - if attribute not found, remove M chars after
    QString pattern = "time //%{attrname?,1} message";
    PatternFormatter formatter(pattern);

    // With attribute set
    auto msg1 = MockLogMessage::create(QtInfoMsg, "test");
    msg1.setAttribute("attrname", "VALUE");
    QString formatted1 = formatter.format(msg1);
    QCOMPARE(formatted1, QString("time //VALUE message"));

    // Without attribute - should remove 1 char after (space after placeholder)
    auto msg2 = MockLogMessage::create(QtInfoMsg, "test");
    QString formatted2 = formatter.format(msg2);
    QCOMPARE(formatted2, QString("time //message"));
}

void TestPatternFormatter::testPatternFormatterWithOptionalAttributeRemoveBeforeAndAfter()
{
    // Test %{attr?N,M} - if attribute not found, remove N chars before and M after
    QString pattern = "time //%{attrname?2,1} message";
    PatternFormatter formatter(pattern);

    // With attribute set
    auto msg1 = MockLogMessage::create(QtInfoMsg, "test");
    msg1.setAttribute("attrname", "VALUE");
    QString formatted1 = formatter.format(msg1);
    QCOMPARE(formatted1, QString("time //VALUE message"));

    // Without attribute - should remove 2 chars before ("//") and 1 after (" ")
    auto msg2 = MockLogMessage::create(QtInfoMsg, "test");
    QString formatted2 = formatter.format(msg2);
    QCOMPARE(formatted2, QString("time message"));
}

void TestPatternFormatter::testPatternFormatterWithLeftAlign()
{
    // Test %{type:<10} - left align with width 10
    QString pattern = "[%{type:<10}] %{message}";
    PatternFormatter formatter(pattern);

    auto msg = MockLogMessage::create(QtInfoMsg, "test");
    QString formatted = formatter.format(msg);

    // "info" is 4 chars, should be padded to 10 with spaces on the right
    QCOMPARE(formatted, QString("[info      ] test"));
}

void TestPatternFormatter::testPatternFormatterWithRightAlign()
{
    // Test %{type:>10} - right align with width 10
    QString pattern = "[%{type:>10}] %{message}";
    PatternFormatter formatter(pattern);

    auto msg = MockLogMessage::create(QtDebugMsg, "test");
    QString formatted = formatter.format(msg);

    // "debug" is 5 chars, should be padded to 10 with spaces on the left
    QCOMPARE(formatted, QString("[     debug] test"));
}

void TestPatternFormatter::testPatternFormatterWithCenterAlign()
{
    // Test %{type:^10} - center align with width 10
    QString pattern = "[%{type:^10}] %{message}";
    PatternFormatter formatter(pattern);

    auto msg = MockLogMessage::create(QtInfoMsg, "test");
    QString formatted = formatter.format(msg);

    // "info" is 4 chars, 6 chars padding: 3 left, 3 right
    QCOMPARE(formatted, QString("[   info   ] test"));
}

void TestPatternFormatter::testPatternFormatterWithCustomFillChar()
{
    // Test %{type:*<10} - left align with * fill character
    QString pattern = "[%{type:*<10}]";
    PatternFormatter formatter(pattern);

    auto msg = MockLogMessage::create(QtInfoMsg, "test");
    QString formatted = formatter.format(msg);

    QCOMPARE(formatted, QString("[info******]"));

    // Test with different fill chars
    QString pattern2 = "[%{type:_>10}]";
    PatternFormatter formatter2(pattern2);
    QString formatted2 = formatter2.format(msg);
    QCOMPARE(formatted2, QString("[______info]"));

    // Test center with custom fill
    QString pattern3 = "[%{type:-^10}]";
    PatternFormatter formatter3(pattern3);
    QString formatted3 = formatter3.format(msg);
    QCOMPARE(formatted3, QString("[---info---]"));
}

void TestPatternFormatter::testPatternFormatterWithWidthSmallerThanContent()
{
    // never truncate, just return full content
    QString pattern = "[%{type:<3}]";
    PatternFormatter formatter(pattern);

    auto msg = MockLogMessage::create(QtWarningMsg, "test");
    QString formatted = formatter.format(msg);

    // "warning" is 7 chars, width is 3, should NOT truncate
    QCOMPARE(formatted, QString("[warning]"));
}

void TestPatternFormatter::testPatternFormatterWithMultipleFormattedTokens()
{
    // Test multiple tokens with different format specs
    QString pattern = "[%{type:<8}] %{category:>15} | %{message}";
    PatternFormatter formatter(pattern);

    auto msg = MockLogMessage::createWithCategory("app.core", QtInfoMsg, "Multiple formats");
    QString formatted = formatter.format(msg);

    // type "info" padded to 8 left-aligned, category "app.core" padded to 15 right-aligned
    QCOMPARE(formatted, QString("[info    ]        app.core | Multiple formats"));
}

void TestPatternFormatter::testPatternFormatterWithTimeAndFormatSpec()
{
    // Test time token with format AND width spec
    QString pattern = "[%{time hh:mm:ss:>12}] %{message}";
    PatternFormatter formatter(pattern);

    auto msg = MockLogMessage::create(QtInfoMsg, "Time test");
    QString formatted = formatter.format(msg);

    // Extract the time part
    QVERIFY(formatted.startsWith("["));
    QVERIFY(formatted.contains("] Time test"));

    // The time format "hh:mm:ss" produces 8 chars, should be right-padded to 12
    int closeBracket = formatted.indexOf(']');
    QString timePart = formatted.mid(1, closeBracket - 1);
    QCOMPARE(timePart.length(), 12);
    // Should have 4 spaces on the left (right-aligned)
    QVERIFY(timePart.startsWith("    "));
}

void TestPatternFormatter::testPatternFormatterWithTruncation()
{
    // Test %{type:<5!} - left align with width 5 and truncation
    QString pattern = "[%{type:<5!}]";
    PatternFormatter formatter(pattern);

    // "warning" is 7 chars, should be truncated to 5
    auto msg = MockLogMessage::create(QtWarningMsg, "test");
    QString formatted = formatter.format(msg);
    QCOMPARE(formatted, QString("[warni]"));

    // "info" is 4 chars, should be padded to 5
    auto msg2 = MockLogMessage::create(QtInfoMsg, "test");
    QString formatted2 = formatter.format(msg2);
    QCOMPARE(formatted2, QString("[info ]"));
}

void TestPatternFormatter::testPatternFormatterWithTruncationAndAlign()
{
    // Test right align with truncation
    QString pattern = "[%{type:>5!}]";
    PatternFormatter formatter(pattern);

    // "warning" truncated to "warni", right-aligned (no padding needed as it fills width)
    auto msg = MockLogMessage::create(QtWarningMsg, "test");
    QString formatted = formatter.format(msg);
    QCOMPARE(formatted, QString("[warni]"));

    // "info" is 4 chars, right-aligned with 1 space padding
    auto msg2 = MockLogMessage::create(QtInfoMsg, "test");
    QString formatted2 = formatter.format(msg2);
    QCOMPARE(formatted2, QString("[ info]"));

    // Test center align with truncation
    QString pattern2 = "[%{type:^5!}]";
    PatternFormatter formatter2(pattern2);

    // "critical" truncated to "criti"
    auto msg3 = MockLogMessage::create(QtCriticalMsg, "test");
    QString formatted3 = formatter2.format(msg3);
    QCOMPARE(formatted3, QString("[criti]"));

    // "info" centered: 1 space left, 0 spaces right (4 chars + 1 padding)
    // Wait, width is 5, "info" is 4, so 1 padding: 0 left, 1 right
    QString formatted4 = formatter2.format(msg2);
    QCOMPARE(formatted4, QString("[info ]"));
}

void TestPatternFormatter::testPatternFormatterWithTruncationAndFillChar()
{
    // Test truncation with custom fill character
    QString pattern = "[%{type:*^10!}]";
    PatternFormatter formatter(pattern);

    // "info" is 4 chars, 6 padding: 3 left, 3 right with '*'
    auto msg = MockLogMessage::create(QtInfoMsg, "test");
    QString formatted = formatter.format(msg);
    QCOMPARE(formatted, QString("[***info***]"));

    // "critical" is 8 chars, truncated to 10 (no truncation needed), then padded
    auto msg2 = MockLogMessage::create(QtCriticalMsg, "test");
    QString formatted2 = formatter.format(msg2);
    QCOMPARE(formatted2, QString("[*critical*]"));

    // Test with a really long message attribute that needs truncation
    QString pattern3 = "[%{myattr:_<8!}]";
    PatternFormatter formatter3(pattern3);

    auto msg3 = MockLogMessage::create(QtInfoMsg, "test");
    msg3.setAttribute("myattr", "verylongvalue");
    QString formatted3 = formatter3.format(msg3);
    QCOMPARE(formatted3, QString("[verylong]"));

    // Shorter value gets padded
    auto msg4 = MockLogMessage::create(QtInfoMsg, "test");
    msg4.setAttribute("myattr", "short");
    QString formatted4 = formatter3.format(msg4);
    QCOMPARE(formatted4, QString("[short___]"));
}

void TestPatternFormatter::testPatternFormatterWithTruncationShorterContent()
{
    // When content is shorter than width, truncation flag has no effect
    QString pattern = "[%{type:<10!}]";
    PatternFormatter formatter(pattern);

    // "info" is 4 chars, should be padded to 10 (truncation doesn't apply)
    auto msg = MockLogMessage::create(QtInfoMsg, "test");
    QString formatted = formatter.format(msg);
    QCOMPARE(formatted, QString("[info      ]"));

    // Exact width match
    QString pattern2 = "[%{type:<4!}]";
    PatternFormatter formatter2(pattern2);
    QString formatted2 = formatter2.format(msg);
    QCOMPARE(formatted2, QString("[info]"));
}

QTEST_MAIN(TestPatternFormatter)
#include "test_patternformatter.moc"

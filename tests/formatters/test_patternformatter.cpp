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

QTEST_MAIN(TestPatternFormatter)
#include "test_patternformatter.moc"
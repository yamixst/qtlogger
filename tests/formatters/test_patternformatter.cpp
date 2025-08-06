#include <QtTest/QtTest>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

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

QTEST_MAIN(TestPatternFormatter)
#include "test_patternformatter.moc"
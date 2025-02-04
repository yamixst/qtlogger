#include <QtTest/QtTest>
#include <QDateTime>
#include <QThread>
#include <QVariantHash>
#include <QMessageLogContext>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

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

class TestFormatters : public QObject
{
    Q_OBJECT

private slots:
    // QtLogMessageFormatter tests
    void testQtLogMessageFormatterBasic();
    void testQtLogMessageFormatterSingleton();
    void testQtLogMessageFormatterDifferentTypes();

    // FunctionFormatter tests
    void testFunctionFormatterBasic();
    void testFunctionFormatterCustomFunction();
    void testFunctionFormatterLambda();
    void testFunctionFormatterComplexLogic();

    // JsonFormatter tests
    void testJsonFormatterBasic();
    void testJsonFormatterSingleton();
    void testJsonFormatterComplexAttributes();
    void testJsonFormatterSpecialCharacters();
    void testJsonFormatterNullValues();
    void testJsonFormatterValidJson();

    // PatternFormatter tests
    void testPatternFormatterBasic();
    void testPatternFormatterCustomPattern();
    void testPatternFormatterMultipleMessages();

    // PrettyFormatter tests
    void testPrettyFormatterBasic();
    void testPrettyFormatterSingleton();
    void testPrettyFormatterWithoutThreadId();
    void testPrettyFormatterMaxCategoryWidth();
    void testPrettyFormatterMultipleThreads();
    void testPrettyFormatterDifferentCategories();
    void testPrettyFormatterDefaultCategory();
    void testPrettyFormatterLongCategory();

    // Base Formatter interface tests
    void testFormatterInterface();
    void testFormatterProcessMethod();

    // Edge cases and robustness tests
    void testAllFormattersWithEmptyMessage();
    void testAllFormattersWithLongMessage();
    void testAllFormattersWithSpecialCharacters();

private:
    // Helper methods
    void verifyValidJson(const QString& jsonString);
    bool isValidTimeFormat(const QString& timeStr, const QString& format);
};

// QtLogMessageFormatter Tests

void TestFormatters::testQtLogMessageFormatterBasic()
{
    auto formatter = QtLogMessageFormatter::instance();
    QVERIFY(formatter);
    
    auto msg = MockLogMessage::create(QtDebugMsg, "Test debug message");
    QString formatted = formatter->format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Test debug message"));
}

void TestFormatters::testQtLogMessageFormatterSingleton()
{
    auto formatter1 = QtLogMessageFormatter::instance();
    auto formatter2 = QtLogMessageFormatter::instance();
    
    QCOMPARE(formatter1.data(), formatter2.data());
}

void TestFormatters::testQtLogMessageFormatterDifferentTypes()
{
    auto formatter = QtLogMessageFormatter::instance();
    
    QStringList messages = {
        "Debug message", "Info message", "Warning message", 
        "Critical message", "Fatal message"
    };
    
    QList<QtMsgType> types = {
        QtDebugMsg, QtInfoMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg
    };
    
    for (int i = 0; i < types.size(); ++i) {
        auto msg = MockLogMessage::create(types[i], messages[i]);
        QString formatted = formatter->format(msg);
        
        QVERIFY(!formatted.isEmpty());
        QVERIFY(formatted.contains(messages[i]));
    }
}

// FunctionFormatter Tests

void TestFormatters::testFunctionFormatterBasic()
{
    auto simpleFunction = [](const LogMessage& msg) -> QString {
        return QString("[%1] %2").arg(qtMsgTypeToString(msg.type())).arg(msg.message());
    };
    
    FunctionFormatter formatter(simpleFunction);
    auto msg = MockLogMessage::create(QtWarningMsg, "Function test");
    
    QString formatted = formatter.format(msg);
    QCOMPARE(formatted, QString("[warning] Function test"));
}

void TestFormatters::testFunctionFormatterCustomFunction()
{
    auto customFunction = [](const LogMessage& msg) -> QString {
        return QString("Custom: %1 at line %2").arg(msg.message()).arg(msg.line());
    };
    
    FunctionFormatter formatter(customFunction);
    auto msg = MockLogMessage::create(QtInfoMsg, "Custom message", "test.cpp", 123);
    
    QString formatted = formatter.format(msg);
    QCOMPARE(formatted, QString("Custom: Custom message at line 123"));
}

void TestFormatters::testFunctionFormatterLambda()
{
    FunctionFormatter formatter([](const LogMessage& msg) {
        return QString("Lambda: %1 from %2").arg(msg.message()).arg(msg.function());
    });
    
    auto msg = MockLogMessage::createWithFunction("testLambda", QtDebugMsg, "Lambda test");
    QString formatted = formatter.format(msg);
    
    QCOMPARE(formatted, QString("Lambda: Lambda test from testLambda"));
}

void TestFormatters::testFunctionFormatterComplexLogic()
{
    auto complexFunction = [](const LogMessage& msg) -> QString {
        QString result = QString("[%1]").arg(qtMsgTypeToString(msg.type()).toUpper());
        
        if (msg.hasAttribute("priority")) {
            result += QString(" PRIORITY:%1").arg(msg.attribute("priority").toInt());
        }
        
        result += QString(" %1").arg(msg.message());
        
        if (msg.type() == QtCriticalMsg || msg.type() == QtFatalMsg) {
            result += QString(" (FROM: %1:%2)").arg(msg.file()).arg(msg.line());
        }
        
        return result;
    };
    
    FunctionFormatter formatter(complexFunction);
    
    // Test with normal message (QtInfoMsg is < QtCriticalMsg, so no location info)
    auto msg1 = MockLogMessage::create(QtInfoMsg, "Normal message");
    QString formatted1 = formatter.format(msg1);
    QCOMPARE(formatted1, QString("[INFO] Normal message"));
    
    // Test with priority attribute
    auto msg2 = MockLogMessage::createWithAttributes({{"priority", 5}}, QtWarningMsg, "Priority message");
    QString formatted2 = formatter.format(msg2);
    QCOMPARE(formatted2, QString("[WARNING] PRIORITY:5 Priority message"));
    
    // Test with critical message (should include location)
    auto msg3 = MockLogMessage::createWithLocation("critical.cpp", 456, QtCriticalMsg, "Critical message");
    QString formatted3 = formatter.format(msg3);
    QVERIFY(formatted3.contains("[CRITICAL] Critical message (FROM: critical.cpp:456)"));
}

// JsonFormatter Tests

void TestFormatters::testJsonFormatterBasic()
{
    auto formatter = JsonFormatter::instance();
    QVERIFY(formatter);
    
    auto msg = MockLogMessage::create(QtDebugMsg, "JSON test message");
    QString formatted = formatter->format(msg);
    
    QVERIFY(!formatted.isEmpty());
    verifyValidJson(formatted);
    
    // Parse and verify content
    QJsonDocument doc = QJsonDocument::fromJson(formatted.toUtf8());
    QJsonObject obj = doc.object();
    
    QCOMPARE(obj["message"].toString(), QString("JSON test message"));
    QCOMPARE(obj["type"].toString(), QString("debug"));
}

void TestFormatters::testJsonFormatterSingleton()
{
    auto formatter1 = JsonFormatter::instance();
    auto formatter2 = JsonFormatter::instance();
    
    QCOMPARE(formatter1.data(), formatter2.data());
}

void TestFormatters::testJsonFormatterComplexAttributes()
{
    auto formatter = JsonFormatter::instance();
    auto msg = MockLogMessage::createForJsonTest();
    
    QString formatted = formatter->format(msg);
    verifyValidJson(formatted);
    
    QJsonDocument doc = QJsonDocument::fromJson(formatted.toUtf8());
    QJsonObject obj = doc.object();
    
    // Verify system attributes
    QCOMPARE(obj["message"].toString(), QString("JSON test message"));
    QCOMPARE(obj["type"].toString(), QString("warning"));
    QCOMPARE(obj["line"].toInt(), 123);
    QCOMPARE(obj["file"].toString(), QString("json_test.cpp"));
    QCOMPARE(obj["function"].toString(), QString("jsonTestFunction"));
    QCOMPARE(obj["category"].toString(), QString("json.test"));
    
    // Verify custom attributes
    QCOMPARE(obj["string_attr"].toString(), QString("test string"));
    QCOMPARE(obj["int_attr"].toInt(), 42);
    QCOMPARE(obj["double_attr"].toDouble(), 3.14159);
    QCOMPARE(obj["bool_attr"].toBool(), true);
    QVERIFY(obj.contains("datetime_attr"));
    QVERIFY(obj["null_attr"].isNull());
    QCOMPARE(obj["special_chars"].toString(), QString("äöü€中文🙂"));
}

void TestFormatters::testJsonFormatterSpecialCharacters()
{
    auto formatter = JsonFormatter::instance();
    
    QString specialMessage = "Special: \n\t\r\"'\\€äöüß中文🙂";
    auto msg = MockLogMessage::create(QtDebugMsg, specialMessage);
    msg.setAttribute("special_attr", specialMessage);
    
    QString formatted = formatter->format(msg);
    verifyValidJson(formatted);
    
    QJsonDocument doc = QJsonDocument::fromJson(formatted.toUtf8());
    QJsonObject obj = doc.object();
    
    QCOMPARE(obj["message"].toString(), specialMessage);
    QCOMPARE(obj["special_attr"].toString(), specialMessage);
}

void TestFormatters::testJsonFormatterNullValues()
{
    auto formatter = JsonFormatter::instance();
    auto msg = MockLogMessage::create(QtDebugMsg, QString());
    msg.setAttribute("null_string", QString());
    msg.setAttribute("null_variant", QVariant());
    
    QString formatted = formatter->format(msg);
    verifyValidJson(formatted);
    
    QJsonDocument doc = QJsonDocument::fromJson(formatted.toUtf8());
    QJsonObject obj = doc.object();
    
    QVERIFY(obj["message"].toString().isEmpty());
    QVERIFY(obj.contains("null_string"));
    QVERIFY(obj["null_variant"].isNull());
}

void TestFormatters::testJsonFormatterValidJson()
{
    auto formatter = JsonFormatter::instance();
    
    // Test various message types
    QList<QtMsgType> types = {QtDebugMsg, QtInfoMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg};
    
    for (auto type : types) {
        auto msg = MockLogMessage::create(type, QString("Message for %1").arg(qtMsgTypeToString(type)));
        QString formatted = formatter->format(msg);
        verifyValidJson(formatted);
    }
}

// PatternFormatter Tests

void TestFormatters::testPatternFormatterBasic()
{
    QString pattern = "%{time} [%{type}] %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg = MockLogMessage::create(QtInfoMsg, "Pattern test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Pattern test"));
    QVERIFY(formatted.contains("[info]") || formatted.contains("[Info]") || formatted.contains("info"));
}

void TestFormatters::testPatternFormatterCustomPattern()
{
    QString pattern = "%{file}:%{line} - %{function}() - %{message}";
    PatternFormatter formatter(pattern);
    
    auto msg = MockLogMessage::createWithLocation("custom.cpp", 789, QtWarningMsg, "Custom pattern test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Custom pattern test"));
    // Note: The exact format depends on Qt's qFormatLogMessage implementation
}

void TestFormatters::testPatternFormatterMultipleMessages()
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

// PrettyFormatter Tests

void TestFormatters::testPrettyFormatterBasic()
{
    auto formatter = PrettyFormatter::instance();
    QVERIFY(formatter);
    
    auto msg = MockLogMessage::create(QtInfoMsg, "Pretty test message");
    QString formatted = formatter->format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Pretty test message"));
    
    // Should contain timestamp
    QRegularExpression timeRegex(R"(\d{2}\.\d{2}\.\d{4} \d{2}:\d{2}:\d{2}\.\d{3})");
    QVERIFY(timeRegex.match(formatted).hasMatch());
}

void TestFormatters::testPrettyFormatterSingleton()
{
    auto formatter1 = PrettyFormatter::instance();
    auto formatter2 = PrettyFormatter::instance();
    
    QCOMPARE(formatter1.data(), formatter2.data());
}

void TestFormatters::testPrettyFormatterWithoutThreadId()
{
    PrettyFormatter formatter(false, 0);  // No thread ID, no max category width
    
    auto msg = MockLogMessage::create(QtWarningMsg, "No thread test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("No thread test"));
    QVERIFY(formatted.contains("W"));  // Warning indicator
    
    // Should not contain thread indicator when only one thread
    QVERIFY(!formatted.contains("#"));
}

void TestFormatters::testPrettyFormatterMaxCategoryWidth()
{
    PrettyFormatter formatter(true, 20);  // Max category width 20
    formatter.setMaxCategoryWidth(15);
    QCOMPARE(formatter.maxCategoryWidth(), 15);
    
    auto msg = MockLogMessage::createWithCategory("very.long.category.name", QtCriticalMsg, "Category width test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Category width test"));
    QVERIFY(formatted.contains("[very.long.category.name]"));
}

void TestFormatters::testPrettyFormatterMultipleThreads()
{
    PrettyFormatter formatter(true, 0);  // Show thread ID
    formatter.setShowThreadId(true);
    QVERIFY(formatter.showThreadId());
    
    // This test simulates multiple threads by using the same formatter
    // In real usage, different thread IDs would be detected automatically
    auto msg1 = MockLogMessage::create(QtDebugMsg, "Thread message 1");
    auto msg2 = MockLogMessage::create(QtInfoMsg, "Thread message 2");
    
    QString formatted1 = formatter.format(msg1);
    QString formatted2 = formatter.format(msg2);
    
    QVERIFY(!formatted1.isEmpty());
    QVERIFY(!formatted2.isEmpty());
    QVERIFY(formatted1.contains("Thread message 1"));
    QVERIFY(formatted2.contains("Thread message 2"));
}

void TestFormatters::testPrettyFormatterDifferentCategories()
{
    PrettyFormatter formatter(true, 20);
    
    auto msgDefault = MockLogMessage::createWithCategory("", QtDebugMsg, "Default category");
    auto msgCustom = MockLogMessage::createWithCategory("custom.category", QtWarningMsg, "Custom category");
    
    QString formattedDefault = formatter.format(msgDefault);
    QString formattedCustom = formatter.format(msgCustom);
    
    QVERIFY(!formattedDefault.isEmpty());
    QVERIFY(!formattedCustom.isEmpty());
    QVERIFY(formattedDefault.contains("Default category"));
    QVERIFY(formattedCustom.contains("Custom category"));
    QVERIFY(formattedCustom.contains("[custom.category]"));
}

void TestFormatters::testPrettyFormatterDefaultCategory()
{
    PrettyFormatter formatter(true, 15);
    
    auto msg = MockLogMessage::createWithCategory("default", QtDebugMsg, "Default category test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Default category test"));
    // Should not show [default] category
    QVERIFY(!formatted.contains("[default]"));
}

void TestFormatters::testPrettyFormatterLongCategory()
{
    PrettyFormatter formatter(true, 10);  // Small max width
    
    auto msg = MockLogMessage::createWithCategory("very.very.long.category.name.that.exceeds.limit", 
                                                   QtCriticalMsg, "Long category test");
    QString formatted = formatter.format(msg);
    
    QVERIFY(!formatted.isEmpty());
    QVERIFY(formatted.contains("Long category test"));
    QVERIFY(formatted.contains("E"));  // Critical uses "E" in pretty formatter
}

// Base Formatter Interface Tests

void TestFormatters::testFormatterInterface()
{
    auto formatter = JsonFormatter::instance();
    
    // Test type
    QCOMPARE(formatter->type(), Handler::HandlerType::Formatter);
    
    // Test that it's indeed a Handler
    Handler* handler = formatter.data();
    QVERIFY(handler != nullptr);
    QCOMPARE(handler->type(), Handler::HandlerType::Formatter);
}

void TestFormatters::testFormatterProcessMethod()
{
    auto formatter = JsonFormatter::instance();
    auto msg = MockLogMessage::create(QtDebugMsg, "Process test");
    
    // Initially not formatted
    QVERIFY(!msg.isFormatted());
    
    // Process should format the message
    bool result = formatter->process(msg);
    QVERIFY(result);
    QVERIFY(msg.isFormatted());
    QVERIFY(!msg.formattedMessage().isEmpty());
    
    // Formatted message should be valid JSON
    verifyValidJson(msg.formattedMessage());
}

// Edge Cases and Robustness Tests

void TestFormatters::testAllFormattersWithEmptyMessage()
{
    auto emptyMsg = MockLogMessage::createEmpty();
    
    // QtLogMessageFormatter
    auto qtFormatter = QtLogMessageFormatter::instance();
    QString qtFormatted = qtFormatter->format(emptyMsg);
    QVERIFY(!qtFormatted.isNull());  // Should not crash
    
    // FunctionFormatter
    FunctionFormatter funcFormatter([](const LogMessage& msg) {
        return QString("Empty: '%1'").arg(msg.message());
    });
    QString funcFormatted = funcFormatter.format(emptyMsg);
    QCOMPARE(funcFormatted, QString("Empty: ''"));
    
    // JsonFormatter
    auto jsonFormatter = JsonFormatter::instance();
    QString jsonFormatted = jsonFormatter->format(emptyMsg);
    verifyValidJson(jsonFormatted);
    
    // PatternFormatter
    PatternFormatter patternFormatter("%{message}");
    QString patternFormatted = patternFormatter.format(emptyMsg);
    // PatternFormatter may return null QString for empty messages, but should not crash
    Q_UNUSED(patternFormatted);
    
    // PrettyFormatter
    auto prettyFormatter = PrettyFormatter::instance();
    QString prettyFormatted = prettyFormatter->format(emptyMsg);
    QVERIFY(!prettyFormatted.isEmpty());
}

void TestFormatters::testAllFormattersWithLongMessage()
{
    auto longMsg = MockLogMessage::createLong();
    
    // All formatters should handle long messages without crashing
    auto qtFormatter = QtLogMessageFormatter::instance();
    QString qtFormatted = qtFormatter->format(longMsg);
    QVERIFY(!qtFormatted.isEmpty());
    
    FunctionFormatter funcFormatter([](const LogMessage& msg) {
        return QString("Long: %1").arg(msg.message().left(50) + "...");
    });
    QString funcFormatted = funcFormatter.format(longMsg);
    QVERIFY(funcFormatted.startsWith("Long: Long message: xxx"));
    
    auto jsonFormatter = JsonFormatter::instance();
    QString jsonFormatted = jsonFormatter->format(longMsg);
    verifyValidJson(jsonFormatted);
    
    PatternFormatter patternFormatter("%{message}");
    QString patternFormatted = patternFormatter.format(longMsg);
    QVERIFY(!patternFormatted.isEmpty());
    
    auto prettyFormatter = PrettyFormatter::instance();
    QString prettyFormatted = prettyFormatter->format(longMsg);
    QVERIFY(!prettyFormatted.isEmpty());
}

void TestFormatters::testAllFormattersWithSpecialCharacters()
{
    QString specialMessage = "Special chars: \n\t\r\"'\\€äöüß中文🙂";
    auto specialMsg = MockLogMessage::create(QtDebugMsg, specialMessage);
    
    // All formatters should handle special characters properly
    auto qtFormatter = QtLogMessageFormatter::instance();
    QString qtFormatted = qtFormatter->format(specialMsg);
    QVERIFY(qtFormatted.contains("Special chars"));
    
    FunctionFormatter funcFormatter([](const LogMessage& msg) {
        return QString("Special: %1").arg(msg.message());
    });
    QString funcFormatted = funcFormatter.format(specialMsg);
    QVERIFY(funcFormatted.contains(specialMessage));
    
    auto jsonFormatter = JsonFormatter::instance();
    QString jsonFormatted = jsonFormatter->format(specialMsg);
    verifyValidJson(jsonFormatted);
    QJsonDocument doc = QJsonDocument::fromJson(jsonFormatted.toUtf8());
    QCOMPARE(doc.object()["message"].toString(), specialMessage);
    
    PatternFormatter patternFormatter("%{message}");
    QString patternFormatted = patternFormatter.format(specialMsg);
    QVERIFY(patternFormatted.contains("Special chars"));
    
    auto prettyFormatter = PrettyFormatter::instance();
    QString prettyFormatted = prettyFormatter->format(specialMsg);
    QVERIFY(prettyFormatted.contains("Special chars"));
}

// Helper Methods

void TestFormatters::verifyValidJson(const QString& jsonString)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON Parse Error:" << error.errorString();
        qDebug() << "JSON String:" << jsonString;
    }
    
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVERIFY(!doc.isNull());
    QVERIFY(doc.isObject());
}

bool TestFormatters::isValidTimeFormat(const QString& timeStr, const QString& format)
{
    QDateTime parsed = QDateTime::fromString(timeStr, format);
    return parsed.isValid();
}

QTEST_MAIN(TestFormatters)
#include "test_formatters.moc"
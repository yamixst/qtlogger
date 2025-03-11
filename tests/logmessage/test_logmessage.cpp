#include <QtTest/QtTest>
#include <QDateTime>
#include <QThread>
#include <QVariantHash>
#include <QMessageLogContext>

#include "qtlogger.h"
#include "mock_context.h"

using namespace QtLogger;

class TestLogMessage : public QObject
{
    Q_OBJECT

private slots:
    // Constructor tests
    void testDefaultConstructor();
    void testParameterizedConstructor();
    void testCopyConstructor();

    // Basic property tests
    void testType();
    void testContext();
    void testMessage();
    void testContextMembers();

    // System attributes tests
    void testTime();
    void testThreadId();

    // Formatted message tests
    void testFormattedMessage();
    void testSetFormattedMessage();
    void testIsFormatted();

    // Custom attributes tests
    void testAttribute();
    void testSetAttribute();
    void testHasAttribute();
    void testAttributes();

    // All attributes tests
    void testAllAttributes();

    // Edge cases and robustness tests
    void testEmptyMessage();
    void testNullAttributes();
    void testLargeMessage();
    void testSpecialCharacters();
    void testMultipleAttributes();

    // Helper function tests
    void testQtMsgTypeToString();
    void testStringToQtMsgType();
};

void TestLogMessage::testDefaultConstructor()
{
    LogMessage msg;
    
    QCOMPARE(msg.type(), QtDebugMsg);
    QVERIFY(msg.message().isEmpty());
    QVERIFY(msg.attributes().isEmpty());
    QVERIFY(!msg.isFormatted());
    QCOMPARE(msg.line(), 0);
    QVERIFY(msg.file() == nullptr || strlen(msg.file()) == 0);
}

void TestLogMessage::testParameterizedConstructor()
{
    auto context = Test::MockContext::create("main.cpp", 123, "myFunction", "app.core");
    QString message = "Test message";
    QtMsgType type = QtWarningMsg;
    
    LogMessage msg(type, context, message);
    
    QCOMPARE(msg.type(), type);
    QCOMPARE(msg.message(), message);
    QCOMPARE(msg.line(), 123);
    QCOMPARE(QString(msg.file()), QString("main.cpp"));
    QCOMPARE(QString(msg.function()), QString("myFunction"));
    QCOMPARE(QString(msg.category()), QString("app.core"));
    
    // System attributes should be set automatically
    QVERIFY(!msg.time().isNull());
    QVERIFY(msg.threadId() != 0);
}

void TestLogMessage::testCopyConstructor()
{
    auto context = Test::MockContext::create("test.cpp", 456, "testFunc", "test.cat");
    QString message = "Original message";
    LogMessage original(QtCriticalMsg, context, message);
    
    // Add some attributes to the original
    original.setAttribute("custom", "value");
    original.setFormattedMessage("Formatted: " + message);
    
    LogMessage copy(original);
    
    // Verify all properties are copied
    QCOMPARE(copy.type(), original.type());
    QCOMPARE(copy.message(), original.message());
    QCOMPARE(copy.line(), original.line());
    QCOMPARE(QString(copy.file()), QString(original.file()));
    QCOMPARE(QString(copy.function()), QString(original.function()));
    QCOMPARE(QString(copy.category()), QString(original.category()));
    QCOMPARE(copy.time(), original.time());
    QCOMPARE(copy.threadId(), original.threadId());
    QCOMPARE(copy.formattedMessage(), original.formattedMessage());
    QCOMPARE(copy.attributes(), original.attributes());
    QCOMPARE(copy.isFormatted(), original.isFormatted());
}

void TestLogMessage::testType()
{
    auto context = Test::MockContext::create();
    
    LogMessage debugMsg(QtDebugMsg, context, "debug");
    QCOMPARE(debugMsg.type(), QtDebugMsg);
    
    LogMessage infoMsg(QtInfoMsg, context, "info");
    QCOMPARE(infoMsg.type(), QtInfoMsg);
    
    LogMessage warningMsg(QtWarningMsg, context, "warning");
    QCOMPARE(warningMsg.type(), QtWarningMsg);
    
    LogMessage criticalMsg(QtCriticalMsg, context, "critical");
    QCOMPARE(criticalMsg.type(), QtCriticalMsg);
    
    LogMessage fatalMsg(QtFatalMsg, context, "fatal");
    QCOMPARE(fatalMsg.type(), QtFatalMsg);
}

void TestLogMessage::testContext()
{
    auto context = Test::MockContext::create("source.cpp", 789, "method", "module.test");
    LogMessage msg(QtInfoMsg, context, "test");
    
    const auto& msgContext = msg.context();
    QCOMPARE(msgContext.line, 789);
    QCOMPARE(QString(msgContext.file), QString("source.cpp"));
    QCOMPARE(QString(msgContext.function), QString("method"));
    QCOMPARE(QString(msgContext.category), QString("module.test"));
}

void TestLogMessage::testMessage()
{
    auto context = Test::MockContext::create();
    QString testMessage = "This is a test message with special chars: äöü €";
    
    LogMessage msg(QtDebugMsg, context, testMessage);
    QCOMPARE(msg.message(), testMessage);
}

void TestLogMessage::testContextMembers()
{
    auto context = Test::MockContext::create("file.h", 999, "function", "category");
    LogMessage msg(QtDebugMsg, context, "test");
    
    QCOMPARE(msg.line(), 999);
    QCOMPARE(QString(msg.file()), QString("file.h"));
    QCOMPARE(QString(msg.function()), QString("function"));
    QCOMPARE(QString(msg.category()), QString("category"));
}

void TestLogMessage::testTime()
{
    auto before = QDateTime::currentDateTime();
    
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, "test");
    
    auto after = QDateTime::currentDateTime();
    
    QVERIFY(msg.time() >= before);
    QVERIFY(msg.time() <= after);
}

void TestLogMessage::testThreadId()
{
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, "test");
    
    auto currentThreadId = reinterpret_cast<qintptr>(QThread::currentThreadId());
    QCOMPARE(msg.threadId(), currentThreadId);
}

void TestLogMessage::testFormattedMessage()
{
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, "original");
    
    // Initially, formatted message should be the same as original
    QCOMPARE(msg.formattedMessage(), msg.message());
    QVERIFY(!msg.isFormatted());
}

void TestLogMessage::testSetFormattedMessage()
{
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, "original");
    QString formatted = "[DEBUG] original message";
    
    msg.setFormattedMessage(formatted);
    
    QCOMPARE(msg.formattedMessage(), formatted);
    QCOMPARE(msg.message(), QString("original")); // Original should remain unchanged
    QVERIFY(msg.isFormatted());
}

void TestLogMessage::testIsFormatted()
{
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, "test");
    
    QVERIFY(!msg.isFormatted());
    
    msg.setFormattedMessage("formatted");
    QVERIFY(msg.isFormatted());
}

void TestLogMessage::testAttribute()
{
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, "test");
    
    // Non-existent attribute should return invalid QVariant
    QVERIFY(!msg.attribute("nonexistent").isValid());
    
    // Set and get attribute
    msg.setAttribute("testKey", "testValue");
    QCOMPARE(msg.attribute("testKey").toString(), QString("testValue"));
    
    // Different types
    msg.setAttribute("intValue", 42);
    QCOMPARE(msg.attribute("intValue").toInt(), 42);
    
    msg.setAttribute("boolValue", true);
    QCOMPARE(msg.attribute("boolValue").toBool(), true);
}

void TestLogMessage::testSetAttribute()
{
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, "test");
    
    msg.setAttribute("key1", "value1");
    msg.setAttribute("key2", 123);
    msg.setAttribute("key3", QDateTime::currentDateTime());
    
    QCOMPARE(msg.attribute("key1").toString(), QString("value1"));
    QCOMPARE(msg.attribute("key2").toInt(), 123);
    QVERIFY(msg.attribute("key3").toDateTime().isValid());
    
    // Overwrite existing attribute
    msg.setAttribute("key1", "newValue");
    QCOMPARE(msg.attribute("key1").toString(), QString("newValue"));
}

void TestLogMessage::testHasAttribute()
{
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, "test");
    
    QVERIFY(!msg.hasAttribute("nonexistent"));
    
    msg.setAttribute("existing", "value");
    QVERIFY(msg.hasAttribute("existing"));
    QVERIFY(!msg.hasAttribute("stillnonexistent"));
}

void TestLogMessage::testAttributes()
{
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, "test");
    
    // Initially empty
    QVERIFY(msg.attributes().isEmpty());
    
    // Add some attributes
    msg.setAttribute("attr1", "value1");
    msg.setAttribute("attr2", 42);
    
    auto attrs = msg.attributes();
    QCOMPARE(attrs.size(), 2);
    QVERIFY(attrs.contains("attr1"));
    QVERIFY(attrs.contains("attr2"));
    QCOMPARE(attrs["attr1"].toString(), QString("value1"));
    QCOMPARE(attrs["attr2"].toInt(), 42);
}

void TestLogMessage::testAllAttributes()
{
    auto context = Test::MockContext::create("test.cpp", 100, "func", "cat");
    LogMessage msg(QtWarningMsg, context, "test message");
    
    // Add custom attributes
    msg.setAttribute("custom1", "value1");
    msg.setAttribute("custom2", 42);
    
    auto allAttrs = msg.allAttributes();
    
    // Check system attributes
    QCOMPARE(allAttrs["type"].toString(), QString("warning"));
    QCOMPARE(allAttrs["line"].toInt(), 100);
    QCOMPARE(allAttrs["file"].toString(), QString("test.cpp"));
    QCOMPARE(allAttrs["function"].toString(), QString("func"));
    QCOMPARE(allAttrs["category"].toString(), QString("cat"));
    QVERIFY(allAttrs.contains("time"));
    QVERIFY(allAttrs.contains("threadId"));
    
    // Check custom attributes
    QCOMPARE(allAttrs["custom1"].toString(), QString("value1"));
    QCOMPARE(allAttrs["custom2"].toInt(), 42);
    
    // Should have at least 9 attributes (7 system + 2 custom)
    QVERIFY(allAttrs.size() >= 9);
}

void TestLogMessage::testEmptyMessage()
{
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, QString());
    
    QVERIFY(msg.message().isEmpty());
    QCOMPARE(msg.formattedMessage(), QString());
}

void TestLogMessage::testNullAttributes()
{
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, "test");
    
    // Set null/invalid attributes
    msg.setAttribute("nullString", QString());
    msg.setAttribute("invalidVariant", QVariant());
    
    QVERIFY(msg.hasAttribute("nullString"));
    QVERIFY(msg.hasAttribute("invalidVariant"));
    QVERIFY(msg.attribute("nullString").toString().isEmpty());
    QVERIFY(!msg.attribute("invalidVariant").isValid());
}

void TestLogMessage::testLargeMessage()
{
    QString largeMessage = QString("x").repeated(10000);
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, largeMessage);
    
    QCOMPARE(msg.message().length(), 10000);
    QCOMPARE(msg.message(), largeMessage);
}

void TestLogMessage::testSpecialCharacters()
{
    QString specialMessage = "Special chars: \n\t\r\"'\\€äöüß中文🙂";
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, specialMessage);
    
    QCOMPARE(msg.message(), specialMessage);
    
    // Test in attributes too
    msg.setAttribute("special", specialMessage);
    QCOMPARE(msg.attribute("special").toString(), specialMessage);
}

void TestLogMessage::testMultipleAttributes()
{
    auto context = Test::MockContext::create();
    LogMessage msg(QtDebugMsg, context, "test");
    
    // Add many attributes
    for (int i = 0; i < 100; ++i) {
        msg.setAttribute(QString("key%1").arg(i), i * 2);
    }
    
    QCOMPARE(msg.attributes().size(), 100);
    
    // Verify some values
    QCOMPARE(msg.attribute("key0").toInt(), 0);
    QCOMPARE(msg.attribute("key50").toInt(), 100);
    QCOMPARE(msg.attribute("key99").toInt(), 198);
}

void TestLogMessage::testQtMsgTypeToString()
{
    QCOMPARE(qtMsgTypeToString(QtDebugMsg), QString("debug"));
    QCOMPARE(qtMsgTypeToString(QtInfoMsg), QString("info"));
    QCOMPARE(qtMsgTypeToString(QtWarningMsg), QString("warning"));
    QCOMPARE(qtMsgTypeToString(QtCriticalMsg), QString("critical"));
    QCOMPARE(qtMsgTypeToString(QtFatalMsg), QString("fatal"));
    
    // Test with custom default
    QCOMPARE(qtMsgTypeToString(static_cast<QtMsgType>(-1), "unknown"), QString("unknown"));
}

void TestLogMessage::testStringToQtMsgType()
{
    QCOMPARE(stringToQtMsgType("debug"), QtDebugMsg);
    QCOMPARE(stringToQtMsgType("info"), QtInfoMsg);
    QCOMPARE(stringToQtMsgType("warning"), QtWarningMsg);
    QCOMPARE(stringToQtMsgType("critical"), QtCriticalMsg);
    QCOMPARE(stringToQtMsgType("fatal"), QtFatalMsg);
    
    // Test with unknown string (should return default)
    QCOMPARE(stringToQtMsgType("unknown"), QtDebugMsg);
    QCOMPARE(stringToQtMsgType("unknown", QtWarningMsg), QtWarningMsg);
    
    // Test case sensitivity
    QCOMPARE(stringToQtMsgType("DEBUG"), QtDebugMsg); // Should return default since it's case-sensitive
    QCOMPARE(stringToQtMsgType("DEBUG", QtInfoMsg), QtInfoMsg);
}

QTEST_MAIN(TestLogMessage)
#include "test_logmessage.moc"
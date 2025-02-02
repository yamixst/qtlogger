#include <QtTest/QtTest>
#include <QHostInfo>
#include <QVariantHash>
#include <QSharedPointer>

#include "attrhandlers/hostinfoattrs.h"
#include "logmessage.h"
#include "mock_qhostinfo.h"

#ifdef QTLOGGER_NETWORK

using namespace QtLogger;
using namespace QtLogger::Test;

class TestHostInfoAttrs : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testAttributes();
    void testAttributesContent();
    void testAttributesTypes();
    void testAttributesNotEmpty();

    // Process method tests (inherited from AttrHandler)
    void testProcessMethod();
    void testProcessAddsAttributes();
    void testProcessReturnsTrue();

    // Handler type tests
    void testHandlerType();

    // Integration tests
    void testWithLogMessage();
    void testMultipleInstances();
    void testAttributesPersistence();

    // Mock data tests
    void testWithMockData();
    void testWithEmptyMockData();
    void testWithSpecialCharacters();

    // Real host data tests
    void testWithRealHostData();

    // Edge cases
    void testAttributeKeys();
    void testAttributeValues();

private:
    HostInfoAttrsPtr m_handler;
};

void TestHostInfoAttrs::initTestCase()
{
    // Nothing special needed for initialization
}

void TestHostInfoAttrs::cleanupTestCase()
{
    MockQHostInfo::resetMockData();
}

void TestHostInfoAttrs::init()
{
    m_handler = QSharedPointer<HostInfoAttrs>::create();
    MockQHostInfo::resetMockData();
}

void TestHostInfoAttrs::cleanup()
{
    m_handler.reset();
    MockQHostInfo::resetMockData();
}

void TestHostInfoAttrs::testAttributes()
{
    QVERIFY(m_handler);
    
    auto attrs = m_handler->attributes();
    QVERIFY(!attrs.isEmpty());
    
    // Should contain the expected key
    QVERIFY(attrs.contains("host_name"));
    
    // Should have exactly 1 attribute
    QCOMPARE(attrs.size(), 1);
}

void TestHostInfoAttrs::testAttributesContent()
{
    auto attrs = m_handler->attributes();
    
    // Test that value is a string
    QVERIFY(attrs["host_name"].canConvert<QString>());
    
    // Host name should be valid (non-null QVariant)
    QVERIFY(attrs["host_name"].isValid());
}

void TestHostInfoAttrs::testAttributesTypes()
{
    auto attrs = m_handler->attributes();
    
    QCOMPARE(attrs["host_name"].type(), QVariant::String);
}

void TestHostInfoAttrs::testAttributesNotEmpty()
{
    auto attrs = m_handler->attributes();
    
    // Host name should at least be a valid QVariant
    QVERIFY(attrs["host_name"].isValid());
    
    // The actual hostname might be empty depending on system configuration,
    // but the QVariant should be valid
    QString hostName = attrs["host_name"].toString();
    QVERIFY(hostName.isNull() == false); // QString should not be null, but can be empty
}

void TestHostInfoAttrs::testProcessMethod()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");
    
    QVERIFY(msg.attributes().isEmpty());
    
    bool result = m_handler->process(msg);
    
    QVERIFY(result);
    QVERIFY(!msg.attributes().isEmpty());
}

void TestHostInfoAttrs::testProcessAddsAttributes()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");
    
    // Add some existing attributes
    msg.setAttribute("existing", "value");
    QCOMPARE(msg.attributes().size(), 1);
    
    m_handler->process(msg);
    
    // Should now have original + host info attributes
    QCOMPARE(msg.attributes().size(), 2);
    QVERIFY(msg.hasAttribute("existing"));
    QVERIFY(msg.hasAttribute("host_name"));
    
    // Original attribute should be preserved
    QCOMPARE(msg.attribute("existing").toString(), QString("value"));
}

void TestHostInfoAttrs::testProcessReturnsTrue()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");
    
    // process() should always return true for AttrHandler
    QVERIFY(m_handler->process(msg));
}

void TestHostInfoAttrs::testHandlerType()
{
    QCOMPARE(m_handler->type(), HandlerType::AttrHandler);
}

void TestHostInfoAttrs::testWithLogMessage()
{
    QMessageLogContext context("hostinfo_test.cpp", 200, "main", "app.main");
    LogMessage msg(QtWarningMsg, context, "Host information test");
    
    m_handler->process(msg);
    
    auto allAttrs = msg.allAttributes();
    
    // Should have system attributes + host info attributes
    QVERIFY(allAttrs.contains("type"));
    QVERIFY(allAttrs.contains("line"));
    QVERIFY(allAttrs.contains("host_name"));
    
    // Host info should be properly integrated
    QCOMPARE(allAttrs["line"].toInt(), 200);
    QCOMPARE(allAttrs["type"].toString(), QString("warning"));
}

void TestHostInfoAttrs::testMultipleInstances()
{
    auto handler1 = QSharedPointer<HostInfoAttrs>::create();
    auto handler2 = QSharedPointer<HostInfoAttrs>::create();
    
    auto attrs1 = handler1->attributes();
    auto attrs2 = handler2->attributes();
    
    // Both should return the same data (same host)
    QCOMPARE(attrs1["host_name"], attrs2["host_name"]);
}

void TestHostInfoAttrs::testAttributesPersistence()
{
    auto attrs1 = m_handler->attributes();
    auto attrs2 = m_handler->attributes();
    
    // Calling attributes() multiple times should return the same data
    QCOMPARE(attrs1, attrs2);
}

void TestHostInfoAttrs::testWithMockData()
{
    MockQHostInfo::setMockHostName("mock-test-host");
    MockQHostInfo::enableMocking();
    
    auto attrs = m_handler->attributes();
    
    QCOMPARE(attrs["host_name"].toString(), QString("mock-test-host"));
}

void TestHostInfoAttrs::testWithEmptyMockData()
{
    MockQHostInfo::setMockHostName("");
    MockQHostInfo::enableMocking();
    
    auto attrs = m_handler->attributes();
    
    QCOMPARE(attrs["host_name"].toString(), QString(""));
    
    // Attribute should still be present, just with empty value
    QCOMPARE(attrs.size(), 1);
    QVERIFY(attrs.contains("host_name"));
}

void TestHostInfoAttrs::testWithSpecialCharacters()
{
    MockQHostInfo::setMockHostName("host-with-äöü-and-中文-🏠");
    MockQHostInfo::enableMocking();
    
    auto attrs = m_handler->attributes();
    
    QCOMPARE(attrs["host_name"].toString(), QString("host-with-äöü-and-中文-🏠"));
}

void TestHostInfoAttrs::testWithRealHostData()
{
    MockQHostInfo::disableMocking();
    
    auto attrs = m_handler->attributes();
    
    // With real QHostInfo data
    QCOMPARE(attrs["host_name"].toString(), QHostInfo::localHostName());
}

void TestHostInfoAttrs::testAttributeKeys()
{
    auto attrs = m_handler->attributes();
    auto keys = attrs.keys();
    
    // Verify exact key names
    QVERIFY(keys.contains("host_name"));
    
    // Verify no unexpected keys
    QCOMPARE(keys.size(), 1);
    QCOMPARE(keys.first(), QString("host_name"));
}

void TestHostInfoAttrs::testAttributeValues()
{
    MockQHostInfo::setMockHostName("test-hostname");
    MockQHostInfo::enableMocking();
    
    auto attrs = m_handler->attributes();
    
    // Test specific value handling
    QVERIFY(!attrs["host_name"].toString().isEmpty());
    QVERIFY(attrs["host_name"].isValid());
    
    // Host name should be the exact mock value
    QCOMPARE(attrs["host_name"].toString(), QString("test-hostname"));
    
    // Test different hostname formats
    MockQHostInfo::setMockHostName("server.example.com");
    auto attrs2 = m_handler->attributes();
    QCOMPARE(attrs2["host_name"].toString(), QString("server.example.com"));
    
    MockQHostInfo::setMockHostName("localhost");
    auto attrs3 = m_handler->attributes();
    QCOMPARE(attrs3["host_name"].toString(), QString("localhost"));
}

QTEST_MAIN(TestHostInfoAttrs)
#include "test_hostinfoattrs.moc"

#else // QTLOGGER_NETWORK

// Dummy test when network support is disabled
class TestHostInfoAttrs : public QObject
{
    Q_OBJECT
private slots:
    void testNetworkDisabled()
    {
        QSKIP("HostInfoAttrs tests skipped - QTLOGGER_NETWORK not defined");
    }
};

QTEST_MAIN(TestHostInfoAttrs)
#include "test_hostinfoattrs.moc"

#endif // QTLOGGER_NETWORK
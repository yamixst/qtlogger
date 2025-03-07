#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QVariantHash>
#include <QSharedPointer>

#include "attrhandlers/appinfoattrs.h"
#include "logmessage.h"
#include "mock_qcoreapplication.h"

using namespace QtLogger;
using namespace QtLogger::Test;

class TestAppInfoAttrs : public QObject
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

    // Real application data tests
    void testWithRealApplicationData();

    // Edge cases
    void testAttributeKeys();
    void testAttributeValues();

private:
    AppInfoAttrsPtr m_handler;
};

void TestAppInfoAttrs::initTestCase()
{
    // Initialize QCoreApplication if not already done
    if (!QCoreApplication::instance()) {
        static int argc = 1;
        static char* argv[] = {"test_appinfoattrs"};
        static QCoreApplication app(argc, argv);
        
        // Set some application properties for testing
        QCoreApplication::setApplicationName("QtLoggerTestApp");
        QCoreApplication::setApplicationVersion("1.2.3");
    }
}

void TestAppInfoAttrs::cleanupTestCase()
{
    MockQCoreApplication::resetMockData();
}

void TestAppInfoAttrs::init()
{
    m_handler = QSharedPointer<AppInfoAttrs>::create();
    MockQCoreApplication::resetMockData();
}

void TestAppInfoAttrs::cleanup()
{
    m_handler.reset();
    MockQCoreApplication::resetMockData();
}

void TestAppInfoAttrs::testAttributes()
{
    QVERIFY(m_handler);
    
    auto attrs = m_handler->attributes();
    QVERIFY(!attrs.isEmpty());
    
    // Should contain all expected keys
    QVERIFY(attrs.contains("app_name"));
    QVERIFY(attrs.contains("app_version"));
    QVERIFY(attrs.contains("app_dir"));
    QVERIFY(attrs.contains("app_path"));
    QVERIFY(attrs.contains("pid"));
    
    // Should have exactly 5 attributes
    QCOMPARE(attrs.size(), 5);
}

void TestAppInfoAttrs::testAttributesContent()
{
    auto attrs = m_handler->attributes();
    
    // Test that values are strings or numbers as expected
    QVERIFY(attrs["app_name"].canConvert<QString>());
    QVERIFY(attrs["app_version"].canConvert<QString>());
    QVERIFY(attrs["app_dir"].canConvert<QString>());
    QVERIFY(attrs["app_path"].canConvert<QString>());
    QVERIFY(attrs["pid"].canConvert<qint64>());
    
    // PID should be a positive number
    auto pid = attrs["pid"].toLongLong();
    QVERIFY(pid > 0);
}

void TestAppInfoAttrs::testAttributesTypes()
{
    auto attrs = m_handler->attributes();
    
    QCOMPARE(attrs["app_name"].type(), QVariant::String);
    QCOMPARE(attrs["app_version"].type(), QVariant::String);
    QCOMPARE(attrs["app_dir"].type(), QVariant::String);
    QCOMPARE(attrs["app_path"].type(), QVariant::String);
    QCOMPARE(attrs["pid"].type(), QVariant::LongLong);
}

void TestAppInfoAttrs::testAttributesNotEmpty()
{
    auto attrs = m_handler->attributes();
    
    // At minimum, we should have a PID
    QVERIFY(attrs["pid"].toLongLong() > 0);
    
    // Other attributes might be empty depending on the application state,
    // but they should at least be valid QVariants
    QVERIFY(attrs["app_name"].isValid());
    QVERIFY(attrs["app_version"].isValid());
    QVERIFY(attrs["app_dir"].isValid());
    QVERIFY(attrs["app_path"].isValid());
}

void TestAppInfoAttrs::testProcessMethod()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");
    
    QVERIFY(msg.attributes().isEmpty());
    
    bool result = m_handler->process(msg);
    
    QVERIFY(result);
    QVERIFY(!msg.attributes().isEmpty());
}

void TestAppInfoAttrs::testProcessAddsAttributes()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");
    
    // Add some existing attributes
    msg.setAttribute("existing", "value");
    QCOMPARE(msg.attributes().size(), 1);
    
    m_handler->process(msg);
    
    // Should now have original + app info attributes
    QVERIFY(msg.attributes().size() > 1);
    QVERIFY(msg.hasAttribute("existing"));
    QVERIFY(msg.hasAttribute("app_name"));
    QVERIFY(msg.hasAttribute("app_version"));
    QVERIFY(msg.hasAttribute("app_dir"));
    QVERIFY(msg.hasAttribute("app_path"));
    QVERIFY(msg.hasAttribute("pid"));
    
    // Original attribute should be preserved
    QCOMPARE(msg.attribute("existing").toString(), QString("value"));
}

void TestAppInfoAttrs::testProcessReturnsTrue()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");
    
    // process() should always return true for AttrHandler
    QVERIFY(m_handler->process(msg));
}

void TestAppInfoAttrs::testHandlerType()
{
    QCOMPARE(m_handler->type(), HandlerType::AttrHandler);
}

void TestAppInfoAttrs::testWithLogMessage()
{
    QMessageLogContext context("appinfo_test.cpp", 200, "main", "app.main");
    LogMessage msg(QtWarningMsg, context, "Application started");
    
    m_handler->process(msg);
    
    auto allAttrs = msg.allAttributes();
    
    // Should have system attributes + app info attributes
    QVERIFY(allAttrs.contains("type"));
    QVERIFY(allAttrs.contains("line"));
    QVERIFY(allAttrs.contains("app_name"));
    QVERIFY(allAttrs.contains("pid"));
    
    // App info should be properly integrated
    QCOMPARE(allAttrs["line"].toInt(), 200);
    QCOMPARE(allAttrs["type"].toString(), QString("warning"));
}

void TestAppInfoAttrs::testMultipleInstances()
{
    auto handler1 = QSharedPointer<AppInfoAttrs>::create();
    auto handler2 = QSharedPointer<AppInfoAttrs>::create();
    
    auto attrs1 = handler1->attributes();
    auto attrs2 = handler2->attributes();
    
    // Both should return the same data (same application)
    QCOMPARE(attrs1["app_name"], attrs2["app_name"]);
    QCOMPARE(attrs1["app_version"], attrs2["app_version"]);
    QCOMPARE(attrs1["app_dir"], attrs2["app_dir"]);
    QCOMPARE(attrs1["app_path"], attrs2["app_path"]);
    QCOMPARE(attrs1["pid"], attrs2["pid"]);
}

void TestAppInfoAttrs::testAttributesPersistence()
{
    auto attrs1 = m_handler->attributes();
    auto attrs2 = m_handler->attributes();
    
    // Calling attributes() multiple times should return the same data
    QCOMPARE(attrs1, attrs2);
}

void TestAppInfoAttrs::testWithMockData()
{
    MockQCoreApplication::setMockData(
        "MockTestApp",
        "2.5.1",
        "/mock/app/directory",
        "/mock/app/directory/mocktestapp",
        98765
    );
    MockQCoreApplication::enableMocking();
    
    auto attrs = m_handler->attributes();
    
    QCOMPARE(attrs["app_name"].toString(), QString("MockTestApp"));
    QCOMPARE(attrs["app_version"].toString(), QString("2.5.1"));
    QCOMPARE(attrs["app_dir"].toString(), QString("/mock/app/directory"));
    QCOMPARE(attrs["app_path"].toString(), QString("/mock/app/directory/mocktestapp"));
    QCOMPARE(attrs["pid"].toLongLong(), 98765LL);
}

void TestAppInfoAttrs::testWithEmptyMockData()
{
    MockQCoreApplication::setMockData("", "", "", "", 0);
    MockQCoreApplication::enableMocking();
    
    auto attrs = m_handler->attributes();
    
    QCOMPARE(attrs["app_name"].toString(), QString(""));
    QCOMPARE(attrs["app_version"].toString(), QString(""));
    QCOMPARE(attrs["app_dir"].toString(), QString(""));
    QCOMPARE(attrs["app_path"].toString(), QString(""));
    QCOMPARE(attrs["pid"].toLongLong(), 0LL);
    
    // All attributes should still be present, just with empty/zero values
    QCOMPARE(attrs.size(), 5);
}

void TestAppInfoAttrs::testWithSpecialCharacters()
{
    MockQCoreApplication::setMockData(
        "App with spaces & special chars äöü",
        "1.0.0-beta+build.123",
        "/path/with spaces/and/ünicöde",
        "/path/with spaces/and/ünicöde/app with spaces",
        42
    );
    MockQCoreApplication::enableMocking();
    
    auto attrs = m_handler->attributes();
    
    QCOMPARE(attrs["app_name"].toString(), QString("App with spaces & special chars äöü"));
    QCOMPARE(attrs["app_version"].toString(), QString("1.0.0-beta+build.123"));
    QCOMPARE(attrs["app_dir"].toString(), QString("/path/with spaces/and/ünicöde"));
    QCOMPARE(attrs["app_path"].toString(), QString("/path/with spaces/and/ünicöde/app with spaces"));
    QCOMPARE(attrs["pid"].toLongLong(), 42LL);
}

void TestAppInfoAttrs::testWithRealApplicationData()
{
    MockQCoreApplication::disableMocking();
    
    auto attrs = m_handler->attributes();
    
    // With real QCoreApplication data
    QCOMPARE(attrs["app_name"].toString(), QCoreApplication::applicationName());
    QCOMPARE(attrs["app_version"].toString(), QCoreApplication::applicationVersion());
    QCOMPARE(attrs["app_dir"].toString(), QCoreApplication::applicationDirPath());
    QCOMPARE(attrs["app_path"].toString(), QCoreApplication::applicationFilePath());
    QCOMPARE(attrs["pid"].toLongLong(), QCoreApplication::applicationPid());
}

void TestAppInfoAttrs::testAttributeKeys()
{
    auto attrs = m_handler->attributes();
    auto keys = attrs.keys();
    
    // Verify exact key names
    QVERIFY(keys.contains("app_name"));
    QVERIFY(keys.contains("app_version"));
    QVERIFY(keys.contains("app_dir"));
    QVERIFY(keys.contains("app_path"));
    QVERIFY(keys.contains("pid"));
    
    // Verify no unexpected keys
    for (const auto& key : keys) {
        QVERIFY(key == "app_name" || 
                key == "app_version" || 
                key == "app_dir" || 
                key == "app_path" || 
                key == "pid");
    }
}

void TestAppInfoAttrs::testAttributeValues()
{
    MockQCoreApplication::setMockData("TestApp", "1.0", "/test", "/test/app", 123);
    MockQCoreApplication::enableMocking();
    
    auto attrs = m_handler->attributes();
    
    // Test specific value handling
    QVERIFY(!attrs["app_name"].toString().isEmpty());
    QVERIFY(attrs["app_name"].isValid());
    
    // PID should be convertible to different number types
    QCOMPARE(attrs["pid"].toInt(), 123);
    QCOMPARE(attrs["pid"].toLongLong(), 123LL);
    QVERIFY(attrs["pid"].canConvert<quint64>());
}

QTEST_MAIN(TestAppInfoAttrs)
#include "test_appinfoattrs.moc"
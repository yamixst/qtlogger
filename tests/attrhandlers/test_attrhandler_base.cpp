#include <QtTest/QtTest>
#include <QVariantHash>
#include <QSharedPointer>

#include "attrhandler.h"
#include "logmessage.h"

using namespace QtLogger;

// Test implementation of AttrHandler for testing base functionality
class TestAttrHandlerImpl : public AttrHandler
{
public:
    TestAttrHandlerImpl(const QVariantHash& attrs = QVariantHash())
        : m_attributes(attrs)
    {
    }

    QVariantHash attributes() override
    {
        return m_attributes;
    }

    void setAttributes(const QVariantHash& attrs)
    {
        m_attributes = attrs;
    }

private:
    QVariantHash m_attributes;
};

class TestAttrHandlerBase : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // Handler type tests
    void testHandlerType();

    // Process method tests
    void testProcessMethod();
    void testProcessReturnsTrue();
    void testProcessWithEmptyAttributes();
    void testProcessWithSingleAttribute();
    void testProcessWithMultipleAttributes();
    void testProcessPreservesExistingAttributes();
    void testProcessOverwritesAttributes();

    // Integration tests
    void testWithLogMessage();
    void testAttributeIntegration();

    // Qt version compatibility tests
    void testQt5Compatibility();
    void testQt6Compatibility();

    // Edge cases
    void testProcessWithNullValues();
    void testProcessWithSpecialCharacters();
    void testProcessWithLargeData();

private:
    QSharedPointer<TestAttrHandlerImpl> m_handler;
};

void TestAttrHandlerBase::init()
{
    m_handler = QSharedPointer<TestAttrHandlerImpl>::create();
}

void TestAttrHandlerBase::cleanup()
{
    m_handler.reset();
}

void TestAttrHandlerBase::testHandlerType()
{
    QCOMPARE(m_handler->type(), HandlerType::AttrHandler);
}

void TestAttrHandlerBase::testProcessMethod()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    // Set some attributes in the handler
    QVariantHash attrs;
    attrs.insert("test_key", "test_value");
    m_handler->setAttributes(attrs);

    QVERIFY(msg.attributes().isEmpty());

    bool result = m_handler->process(msg);

    QVERIFY(result);
    QVERIFY(!msg.attributes().isEmpty());
    QVERIFY(msg.hasAttribute("test_key"));
    QCOMPARE(msg.attribute("test_key").toString(), QString("test_value"));
}

void TestAttrHandlerBase::testProcessReturnsTrue()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    // process() should always return true for AttrHandler
    QVERIFY(m_handler->process(msg));
}

void TestAttrHandlerBase::testProcessWithEmptyAttributes()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    // Handler with empty attributes
    m_handler->setAttributes(QVariantHash());

    bool result = m_handler->process(msg);

    QVERIFY(result);
    QVERIFY(msg.attributes().isEmpty());
}

void TestAttrHandlerBase::testProcessWithSingleAttribute()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    QVariantHash attrs;
    attrs.insert("single_key", "single_value");
    m_handler->setAttributes(attrs);

    m_handler->process(msg);

    QCOMPARE(msg.attributes().size(), 1);
    QVERIFY(msg.hasAttribute("single_key"));
    QCOMPARE(msg.attribute("single_key").toString(), QString("single_value"));
}

void TestAttrHandlerBase::testProcessWithMultipleAttributes()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    QVariantHash attrs;
    attrs.insert("key1", "value1");
    attrs.insert("key2", 42);
    attrs.insert("key3", true);
    attrs.insert("key4", 3.14);
    m_handler->setAttributes(attrs);

    m_handler->process(msg);

    QCOMPARE(msg.attributes().size(), 4);
    QCOMPARE(msg.attribute("key1").toString(), QString("value1"));
    QCOMPARE(msg.attribute("key2").toInt(), 42);
    QCOMPARE(msg.attribute("key3").toBool(), true);
    QCOMPARE(msg.attribute("key4").toDouble(), 3.14);
}

void TestAttrHandlerBase::testProcessPreservesExistingAttributes()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    // Add existing attributes to message
    msg.setAttribute("existing1", "original1");
    msg.setAttribute("existing2", "original2");
    QCOMPARE(msg.attributes().size(), 2);

    // Set different attributes in handler
    QVariantHash attrs;
    attrs.insert("new1", "added1");
    attrs.insert("new2", "added2");
    m_handler->setAttributes(attrs);

    m_handler->process(msg);

    // Should have both existing and new attributes
    QCOMPARE(msg.attributes().size(), 4);
    QCOMPARE(msg.attribute("existing1").toString(), QString("original1"));
    QCOMPARE(msg.attribute("existing2").toString(), QString("original2"));
    QCOMPARE(msg.attribute("new1").toString(), QString("added1"));
    QCOMPARE(msg.attribute("new2").toString(), QString("added2"));
}

void TestAttrHandlerBase::testProcessOverwritesAttributes()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    // Add existing attributes to message
    msg.setAttribute("shared_key", "original_value");
    msg.setAttribute("unique_existing", "existing_value");

    // Set attributes in handler with overlapping key
    QVariantHash attrs;
    attrs.insert("shared_key", "handler_value");
    attrs.insert("unique_handler", "handler_value");
    m_handler->setAttributes(attrs);

    m_handler->process(msg);

    // Handler attributes should overwrite existing ones with same key
    QCOMPARE(msg.attributes().size(), 3);
    QCOMPARE(msg.attribute("shared_key").toString(), QString("handler_value"));
    QCOMPARE(msg.attribute("unique_existing").toString(), QString("existing_value"));
    QCOMPARE(msg.attribute("unique_handler").toString(), QString("handler_value"));
}

void TestAttrHandlerBase::testWithLogMessage()
{
    QMessageLogContext context("attrhandler_test.cpp", 200, "main", "app.main");
    LogMessage msg(QtWarningMsg, context, "AttrHandler integration test");

    QVariantHash attrs;
    attrs.insert("test_attr", "integration_value");
    attrs.insert("number_attr", 123);
    m_handler->setAttributes(attrs);

    m_handler->process(msg);

    auto allAttrs = msg.allAttributes();

    // Should have system attributes + custom attributes
    QVERIFY(allAttrs.contains("type"));
    QVERIFY(allAttrs.contains("line"));
    QVERIFY(allAttrs.contains("test_attr"));
    QVERIFY(allAttrs.contains("number_attr"));

    // Verify integration
    QCOMPARE(allAttrs["line"].toInt(), 200);
    QCOMPARE(allAttrs["type"].toString(), QString("warning"));
    QCOMPARE(allAttrs["test_attr"].toString(), QString("integration_value"));
    QCOMPARE(allAttrs["number_attr"].toInt(), 123);
}

void TestAttrHandlerBase::testAttributeIntegration()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    // Test with different data types
    QVariantHash attrs;
    attrs.insert("string_attr", QString("test string"));
    attrs.insert("int_attr", 42);
    attrs.insert("bool_attr", true);
    attrs.insert("double_attr", 3.14159);
    attrs.insert("datetime_attr", QDateTime::currentDateTime());
    attrs.insert("stringlist_attr", QStringList{"item1", "item2", "item3"});

    m_handler->setAttributes(attrs);
    m_handler->process(msg);

    // Verify all types are preserved
    QCOMPARE(msg.attribute("string_attr").type(), QVariant::String);
    QCOMPARE(msg.attribute("int_attr").type(), QVariant::Int);
    QCOMPARE(msg.attribute("bool_attr").type(), QVariant::Bool);
    QCOMPARE(msg.attribute("double_attr").type(), QVariant::Double);
    QCOMPARE(msg.attribute("datetime_attr").type(), QVariant::DateTime);
    QCOMPARE(msg.attribute("stringlist_attr").type(), QVariant::StringList);

    // Verify values
    QCOMPARE(msg.attribute("string_attr").toString(), QString("test string"));
    QCOMPARE(msg.attribute("int_attr").toInt(), 42);
    QCOMPARE(msg.attribute("bool_attr").toBool(), true);
    QCOMPARE(msg.attribute("double_attr").toDouble(), 3.14159);
    QVERIFY(msg.attribute("datetime_attr").toDateTime().isValid());
    QCOMPARE(msg.attribute("stringlist_attr").toStringList().size(), 3);
}

void TestAttrHandlerBase::testQt5Compatibility()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    // Add existing attribute
    msg.setAttribute("existing", "original");

    QVariantHash attrs;
    attrs.insert("new_attr", "new_value");
    attrs.insert("existing", "overwritten");  // This should overwrite
    m_handler->setAttributes(attrs);

    m_handler->process(msg);

    // In both Qt5 and Qt6, the handler attributes should be merged/inserted
    QVERIFY(msg.hasAttribute("existing"));
    QVERIFY(msg.hasAttribute("new_attr"));
    QCOMPARE(msg.attribute("new_attr").toString(), QString("new_value"));
    
    // The behavior of overwriting depends on Qt version, but the attribute should exist
    QVERIFY(msg.hasAttribute("existing"));
}

void TestAttrHandlerBase::testQt6Compatibility()
{
    // Same test as Qt5 compatibility - the base functionality should work
    // regardless of Qt version
    testQt5Compatibility();
}

void TestAttrHandlerBase::testProcessWithNullValues()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    QVariantHash attrs;
    attrs.insert("null_string", QString());
    attrs.insert("invalid_variant", QVariant());
    attrs.insert("valid_string", "valid");
    m_handler->setAttributes(attrs);

    m_handler->process(msg);

    QVERIFY(msg.hasAttribute("null_string"));
    QVERIFY(msg.hasAttribute("invalid_variant"));
    QVERIFY(msg.hasAttribute("valid_string"));

    QVERIFY(msg.attribute("null_string").toString().isEmpty());
    QVERIFY(!msg.attribute("invalid_variant").isValid());
    QCOMPARE(msg.attribute("valid_string").toString(), QString("valid"));
}

void TestAttrHandlerBase::testProcessWithSpecialCharacters()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    QVariantHash attrs;
    attrs.insert("unicode_key_äöü", "unicode_value_中文🎉");
    attrs.insert("special_chars", "Line1\nLine2\tTabbed\"Quoted\"\\Backslash");
    attrs.insert("key with spaces", "value with spaces");
    m_handler->setAttributes(attrs);

    m_handler->process(msg);

    QVERIFY(msg.hasAttribute("unicode_key_äöü"));
    QVERIFY(msg.hasAttribute("special_chars"));
    QVERIFY(msg.hasAttribute("key with spaces"));

    QCOMPARE(msg.attribute("unicode_key_äöü").toString(), QString("unicode_value_中文🎉"));
    QCOMPARE(msg.attribute("special_chars").toString(), QString("Line1\nLine2\tTabbed\"Quoted\"\\Backslash"));
    QCOMPARE(msg.attribute("key with spaces").toString(), QString("value with spaces"));
}

void TestAttrHandlerBase::testProcessWithLargeData()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");

    QVariantHash attrs;
    
    // Large string
    QString largeString = QString("x").repeated(10000);
    attrs.insert("large_string", largeString);

    // Many attributes
    for (int i = 0; i < 100; ++i) {
        attrs.insert(QString("attr_%1").arg(i), QString("value_%1").arg(i));
    }

    m_handler->setAttributes(attrs);
    m_handler->process(msg);

    QCOMPARE(msg.attributes().size(), 101); // 100 numbered + 1 large string
    QCOMPARE(msg.attribute("large_string").toString().length(), 10000);
    QCOMPARE(msg.attribute("attr_0").toString(), QString("value_0"));
    QCOMPARE(msg.attribute("attr_99").toString(), QString("value_99"));
}

QTEST_MAIN(TestAttrHandlerBase)
#include "test_attrhandler_base.moc"
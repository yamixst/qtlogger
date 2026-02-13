// SPDX-FileCopyrightText: 2026 Xstream
// SPDX-License-Identifier: MIT

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QSettings>
#include <QUuid>

#include "qtlogger/attrhandlers/appuuidattr.h"
#include "qtlogger/logmessage.h"

using namespace QtLogger;

class TestAppUuidAttr : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testDefaultAttributeName();
    void testCustomAttributeName();
    void testUuidFormat();
    void testUuidPersistence();
    void testUuidConsistency();
    void testMultipleInstances();
    void testEmptyOrganizationName();
    void testEmptyApplicationName();

private:
    void clearStoredUuid();
    QString m_originalOrgName;
    QString m_originalAppName;
};

void TestAppUuidAttr::initTestCase()
{
    m_originalOrgName = QCoreApplication::organizationName();
    m_originalAppName = QCoreApplication::applicationName();
}

void TestAppUuidAttr::cleanupTestCase()
{
    QCoreApplication::setOrganizationName(m_originalOrgName);
    QCoreApplication::setApplicationName(m_originalAppName);
}

void TestAppUuidAttr::init()
{
    QCoreApplication::setOrganizationName("QtLoggerTest");
    QCoreApplication::setApplicationName("TestAppUuidAttr");
    clearStoredUuid();
}

void TestAppUuidAttr::cleanup()
{
    clearStoredUuid();
}

void TestAppUuidAttr::clearStoredUuid()
{
    QSettings settings(QSettings::UserScope, QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());
    settings.remove("app_uuid");
    settings.sync();
}

void TestAppUuidAttr::testDefaultAttributeName()
{
    AppUuidAttr attr;
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");

    auto attrs = attr.attributes(msg);

    QVERIFY(attrs.contains("app_uuid"));
    QVERIFY(!attrs.value("app_uuid").toString().isEmpty());
}

void TestAppUuidAttr::testCustomAttributeName()
{
    AppUuidAttr attr("custom_uuid_name");
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");

    auto attrs = attr.attributes(msg);

    QVERIFY(attrs.contains("custom_uuid_name"));
    QVERIFY(!attrs.contains("app_uuid"));
    QVERIFY(!attrs.value("custom_uuid_name").toString().isEmpty());
}

void TestAppUuidAttr::testUuidFormat()
{
    AppUuidAttr attr;
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");

    auto attrs = attr.attributes(msg);
    auto uuid = attrs.value("app_uuid").toString();

    // UUID should not contain braces
    QVERIFY(!uuid.contains('{'));
    QVERIFY(!uuid.contains('}'));

    // UUID should have correct length (36 chars with dashes, or 32 without)
    QVERIFY(uuid.length() == 36 || uuid.length() == 32);

    // UUID should be parseable
    auto parsedUuid = QUuid::fromString(uuid);
    QVERIFY(!parsedUuid.isNull());
}

void TestAppUuidAttr::testUuidPersistence()
{
    QString uuid1;

    // Create first instance and get UUID
    {
        AppUuidAttr attr;
        LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
        auto attrs = attr.attributes(msg);
        uuid1 = attrs.value("app_uuid").toString();
    }

    // Verify UUID is stored in QSettings
    QSettings settings(QSettings::UserScope, QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());
    auto storedUuid = settings.value("app_uuid").toString();
    QCOMPARE(storedUuid, uuid1);

    // Create new instance - should read same UUID from QSettings
    {
        AppUuidAttr attr;
        LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
        auto attrs = attr.attributes(msg);
        auto uuid2 = attrs.value("app_uuid").toString();
        QCOMPARE(uuid2, uuid1);
    }
}

void TestAppUuidAttr::testUuidConsistency()
{
    AppUuidAttr attr;

    LogMessage msg1(QtDebugMsg, QMessageLogContext(), "message 1");
    LogMessage msg2(QtWarningMsg, QMessageLogContext(), "message 2");
    LogMessage msg3(QtCriticalMsg, QMessageLogContext(), "message 3");

    auto attrs1 = attr.attributes(msg1);
    auto attrs2 = attr.attributes(msg2);
    auto attrs3 = attr.attributes(msg3);

    auto uuid1 = attrs1.value("app_uuid").toString();
    auto uuid2 = attrs2.value("app_uuid").toString();
    auto uuid3 = attrs3.value("app_uuid").toString();

    // All UUIDs should be the same
    QCOMPARE(uuid2, uuid1);
    QCOMPARE(uuid3, uuid1);
}

void TestAppUuidAttr::testMultipleInstances()
{
    // All instances should return the same UUID (from QSettings)
    AppUuidAttr attr1;
    AppUuidAttr attr2("custom_name");
    AppUuidAttr attr3;

    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test");

    auto uuid1 = attr1.attributes(msg).value("app_uuid").toString();
    auto uuid2 = attr2.attributes(msg).value("custom_name").toString();
    auto uuid3 = attr3.attributes(msg).value("app_uuid").toString();

    // All should have the same UUID value
    QCOMPARE(uuid2, uuid1);
    QCOMPARE(uuid3, uuid1);
}

void TestAppUuidAttr::testEmptyOrganizationName()
{
    QCoreApplication::setOrganizationName("");

    // Should still work, just uses empty org name for QSettings
    AppUuidAttr attr;
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test");

    auto attrs = attr.attributes(msg);
    QVERIFY(attrs.contains("app_uuid"));
    QVERIFY(!attrs.value("app_uuid").toString().isEmpty());
}

void TestAppUuidAttr::testEmptyApplicationName()
{
    QCoreApplication::setApplicationName("");

    // Should still work, just uses empty app name for QSettings
    AppUuidAttr attr;
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test");

    auto attrs = attr.attributes(msg);
    QVERIFY(attrs.contains("app_uuid"));
    QVERIFY(!attrs.value("app_uuid").toString().isEmpty());
}

QTEST_MAIN(TestAppUuidAttr)
#include "test_appuuidattr.moc"
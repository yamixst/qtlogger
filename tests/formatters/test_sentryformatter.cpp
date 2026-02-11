// Copyright (C) 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "qtlogger.h"
#include "mock_logmessage.h"

#include "qtlogger/formatters/sentryformatter.h"

using namespace QtLogger;
using namespace QtLogger::Test;

class TestSentryFormatter : public QObject
{
    Q_OBJECT

private slots:
    void testBasicFormat();
    void testEventIdFormat();
    void testTimestampFormat();
    void testSeverityLevels();
    void testMessageFormat();
    void testCategoryAsLogger();
    void testDefaultCategory();
    void testCulpritFromFunction();
    void testTagsWithAppInfo();
    void testExtraContext();
    void testOsContextFromAttributes();
    void testDeviceContextFromAttributes();
    void testRuntimeContext();
    void testSdkInfo();
    void testCustomSdkInfo();
    void testFingerprint();
    void testValidJson();
    void testInstance();
    void testProcessMethod();
    void testEmptyMessage();
    void testSpecialCharacters();
    void testAttributesExcludedFromExtra();

private:
    QJsonObject parseJson(const QString &jsonString);
};

QJsonObject TestSentryFormatter::parseJson(const QString &jsonString)
{
    auto doc = QJsonDocument::fromJson(jsonString.toUtf8());
    return doc.object();
}

void TestSentryFormatter::testBasicFormat()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::create(QtWarningMsg, "Test warning message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(!json.isEmpty());
    QVERIFY(json.contains("event_id"));
    QVERIFY(json.contains("timestamp"));
    QVERIFY(json.contains("platform"));
    QVERIFY(json.contains("level"));
    QVERIFY(json.contains("message"));
    QCOMPARE(json["platform"].toString(), QString("native"));
}

void TestSentryFormatter::testEventIdFormat()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::create();

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    auto eventId = json["event_id"].toString();
    QCOMPARE(eventId.length(), 32); // UUID without dashes is 32 chars
    QVERIFY(!eventId.contains('-'));
    QVERIFY(!eventId.contains('{'));
    QVERIFY(!eventId.contains('}'));
}

void TestSentryFormatter::testTimestampFormat()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::create();

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    auto timestamp = json["timestamp"].toString();
    QVERIFY(!timestamp.isEmpty());

    // Should be ISO 8601 format
    auto parsed = QDateTime::fromString(timestamp, Qt::ISODate);
    QVERIFY(parsed.isValid());
}

void TestSentryFormatter::testSeverityLevels()
{
    SentryFormatter formatter;

    // Test all Qt message types
    struct TestCase {
        QtMsgType type;
        QString expectedLevel;
    };

    QList<TestCase> testCases = {
        { QtDebugMsg, "debug" },
        { QtInfoMsg, "info" },
        { QtWarningMsg, "warning" },
        { QtCriticalMsg, "error" },
        { QtFatalMsg, "fatal" }
    };

    for (const auto &tc : testCases) {
        auto msg = MockLogMessage::create(tc.type, "Test message");
        auto formatted = formatter.format(msg);
        auto json = parseJson(formatted);

        QCOMPARE(json["level"].toString(), tc.expectedLevel);
    }
}

void TestSentryFormatter::testMessageFormat()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::create(QtWarningMsg, "Test warning message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(json.contains("message"));
    auto messageObj = json["message"].toObject();
    QCOMPARE(messageObj["formatted"].toString(), QString("Test warning message"));
}

void TestSentryFormatter::testCategoryAsLogger()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::createWithCategory("custom.category", QtWarningMsg, "Test message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(json.contains("logger"));
    QCOMPARE(json["logger"].toString(), QString("custom.category"));
}

void TestSentryFormatter::testDefaultCategory()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::createWithCategory("default", QtWarningMsg, "Test message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    // Default category should not be included as logger
    QVERIFY(!json.contains("logger"));
}

void TestSentryFormatter::testCulpritFromFunction()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::createWithFunction("myFunction", QtWarningMsg, "Test message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(json.contains("culprit"));
    QCOMPARE(json["culprit"].toString(), QString("myFunction"));
}

void TestSentryFormatter::testTagsWithAppInfo()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::createWithAttributes({
        { "appname", "TestApp" },
        { "appversion", "1.2.3" }
    }, QtWarningMsg, "Test message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(json.contains("tags"));
    auto tags = json["tags"].toObject();

    QVERIFY(tags.contains("qt_version"));
    QCOMPARE(tags["app_name"].toString(), QString("TestApp"));
    QCOMPARE(tags["app_version"].toString(), QString("1.2.3"));
}

void TestSentryFormatter::testExtraContext()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::createWithLocation("test_file.cpp", 42, QtWarningMsg, "Test message");
    msg.setAttribute("custom_attr", "custom_value");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(json.contains("extra"));
    auto extra = json["extra"].toObject();

    QCOMPARE(extra["line"].toInt(), 42);
    QCOMPARE(extra["file"].toString(), QString("test_file.cpp"));
    QVERIFY(extra.contains("thread_id"));
    QCOMPARE(extra["custom_attr"].toString(), QString("custom_value"));
}

void TestSentryFormatter::testOsContextFromAttributes()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::createWithAttributes({
        { "os_name", "linux" },
        { "os_version", "22.04" },
        { "kernel_version", "5.15.0" },
        { "build_abi", "x86_64-linux-gnu" }
    }, QtWarningMsg, "Test message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(json.contains("contexts"));
    auto contexts = json["contexts"].toObject();
    QVERIFY(contexts.contains("os"));

    auto osContext = contexts["os"].toObject();
    QCOMPARE(osContext["name"].toString(), QString("linux"));
    QCOMPARE(osContext["version"].toString(), QString("22.04"));
    QCOMPARE(osContext["kernel_version"].toString(), QString("5.15.0"));
    QCOMPARE(osContext["build"].toString(), QString("x86_64-linux-gnu"));
}

void TestSentryFormatter::testDeviceContextFromAttributes()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::createWithAttributes({
        { "cpu_arch", "x86_64" },
        { "host_name", "testhost" }
    }, QtWarningMsg, "Test message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(json.contains("contexts"));
    auto contexts = json["contexts"].toObject();
    QVERIFY(contexts.contains("device"));

    auto deviceContext = contexts["device"].toObject();
    QCOMPARE(deviceContext["arch"].toString(), QString("x86_64"));
    QCOMPARE(deviceContext["name"].toString(), QString("testhost"));
}

void TestSentryFormatter::testRuntimeContext()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::create(QtWarningMsg, "Test message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(json.contains("contexts"));
    auto contexts = json["contexts"].toObject();
    QVERIFY(contexts.contains("runtime"));

    auto runtimeContext = contexts["runtime"].toObject();
    QCOMPARE(runtimeContext["name"].toString(), QString("Qt"));
    QVERIFY(!runtimeContext["version"].toString().isEmpty());
}

void TestSentryFormatter::testSdkInfo()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::create(QtWarningMsg, "Test message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(json.contains("sdk"));
    auto sdk = json["sdk"].toObject();
    QCOMPARE(sdk["name"].toString(), QString("qtlogger.sentry"));
    QCOMPARE(sdk["version"].toString(), QString("1.0.0"));
}

void TestSentryFormatter::testCustomSdkInfo()
{
    SentryFormatter formatter("my-custom-sdk", "2.0.0");
    auto msg = MockLogMessage::create(QtWarningMsg, "Test message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(json.contains("sdk"));
    auto sdk = json["sdk"].toObject();
    QCOMPARE(sdk["name"].toString(), QString("my-custom-sdk"));
    QCOMPARE(sdk["version"].toString(), QString("2.0.0"));
}

void TestSentryFormatter::testFingerprint()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::createWithCategory("test.category", QtWarningMsg, "Test message for fingerprint");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(json.contains("fingerprint"));
    auto fingerprint = json["fingerprint"].toArray();
    QCOMPARE(fingerprint.size(), 3);
    QCOMPARE(fingerprint[0].toString(), QString("warning"));
    QCOMPARE(fingerprint[1].toString(), QString("test.category"));
    QCOMPARE(fingerprint[2].toString(), QString("Test message for fingerprint"));
}

void TestSentryFormatter::testValidJson()
{
    SentryFormatter formatter;

    QList<QtMsgType> types = { QtDebugMsg, QtInfoMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg };

    for (auto type : types) {
        auto msg = MockLogMessage::create(type, "Test message");
        auto formatted = formatter.format(msg);

        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(formatted.toUtf8(), &error);
        QCOMPARE(error.error, QJsonParseError::NoError);
        QVERIFY(!doc.isNull());
        QVERIFY(doc.isObject());
    }
}

void TestSentryFormatter::testInstance()
{
    auto instance1 = SentryFormatter::instance();
    auto instance2 = SentryFormatter::instance();

    QVERIFY(instance1 != nullptr);
    QCOMPARE(instance1.data(), instance2.data());
}

void TestSentryFormatter::testProcessMethod()
{
    auto formatter = SentryFormatterPtr::create();
    auto msg = MockLogMessage::create(QtWarningMsg, "Test message");

    QVERIFY(!msg.isFormatted());

    bool result = formatter->process(msg);

    QVERIFY(result);
    QVERIFY(msg.isFormatted());
    QVERIFY(!msg.formattedMessage().isEmpty());

    // Verify formatted message is valid JSON
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(msg.formattedMessage().toUtf8(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
}

void TestSentryFormatter::testEmptyMessage()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::create(QtWarningMsg, "");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    QVERIFY(!json.isEmpty());
    auto messageObj = json["message"].toObject();
    QCOMPARE(messageObj["formatted"].toString(), QString(""));
}

void TestSentryFormatter::testSpecialCharacters()
{
    SentryFormatter formatter;
    QString specialMessage = "Test with special chars: äöü€中文🙂\n\t\"quotes\"";
    auto msg = MockLogMessage::create(QtWarningMsg, specialMessage);

    auto formatted = formatter.format(msg);

    // Should be valid JSON
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(formatted.toUtf8(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);

    auto json = doc.object();
    auto messageObj = json["message"].toObject();
    QCOMPARE(messageObj["formatted"].toString(), specialMessage);
}

void TestSentryFormatter::testAttributesExcludedFromExtra()
{
    SentryFormatter formatter;
    auto msg = MockLogMessage::createWithAttributes({
        { "appname", "TestApp" },
        { "appversion", "1.0.0" },
        { "os_name", "linux" },
        { "os_version", "22.04" },
        { "kernel_version", "5.15.0" },
        { "build_abi", "x86_64" },
        { "cpu_arch", "x86_64" },
        { "host_name", "testhost" },
        { "custom_attr", "should_be_in_extra" }
    }, QtWarningMsg, "Test message");

    auto formatted = formatter.format(msg);
    auto json = parseJson(formatted);

    auto extra = json["extra"].toObject();

    // These should NOT be in extra (handled specially)
    QVERIFY(!extra.contains("appname"));
    QVERIFY(!extra.contains("appversion"));
    QVERIFY(!extra.contains("os_name"));
    QVERIFY(!extra.contains("os_version"));
    QVERIFY(!extra.contains("kernel_version"));
    QVERIFY(!extra.contains("build_abi"));
    QVERIFY(!extra.contains("cpu_arch"));
    QVERIFY(!extra.contains("host_name"));

    // Custom attributes should be in extra
    QCOMPARE(extra["custom_attr"].toString(), QString("should_be_in_extra"));
}

QTEST_MAIN(TestSentryFormatter)
#include "test_sentryformatter.moc"
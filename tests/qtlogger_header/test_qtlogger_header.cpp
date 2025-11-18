// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include <QtTest>

// Include the generated single-header file
#include "qtlogger.h"

using namespace QtLogger;

class TestQtloggerHeader : public QObject
{
    Q_OBJECT

private slots:
    void testVersion();
    void testLogMessageCreation();
    void testPipelineCreation();
    void testStdOutSink();
    void testPatternFormatter();
    void testLevelFilter();
    void testBasicLogging();
};

// Helper macro to stringify QTLOGGER_VERSION
#define QTLOGGER_STRINGIFY(x) #x
#define QTLOGGER_VERSION_STRING(x) QTLOGGER_STRINGIFY(x)

void TestQtloggerHeader::testVersion()
{
    // Test that QTLOGGER_VERSION is defined
    QString version = QStringLiteral(QTLOGGER_VERSION_STRING(QTLOGGER_VERSION));
    QVERIFY(!version.isEmpty());
    QVERIFY(version.contains('.')); // Should be in format x.y.z
}

void TestQtloggerHeader::testLogMessageCreation()
{
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "Test message");
    QCOMPARE(msg.type(), QtDebugMsg);
    QCOMPARE(msg.message(), QString("Test message"));
}

void TestQtloggerHeader::testPipelineCreation()
{
    auto pipeline = PipelinePtr::create();
    QVERIFY(!pipeline.isNull());
}

void TestQtloggerHeader::testStdOutSink()
{
    auto sink = StdOutSinkPtr::create();
    QVERIFY(!sink.isNull());
}

void TestQtloggerHeader::testPatternFormatter()
{
    auto formatter = PatternFormatterPtr::create("%{message}");
    QVERIFY(!formatter.isNull());

    LogMessage msg(QtDebugMsg, QMessageLogContext(), "Hello World");
    QString result = formatter->format(msg);
    QCOMPARE(result, QString("Hello World"));
}

void TestQtloggerHeader::testLevelFilter()
{
    auto filter = LevelFilterPtr::create(QtWarningMsg);
    QVERIFY(!filter.isNull());

    LogMessage debugMsg(QtDebugMsg, QMessageLogContext(), "Debug");
    LogMessage warningMsg(QtWarningMsg, QMessageLogContext(), "Warning");
    LogMessage criticalMsg(QtCriticalMsg, QMessageLogContext(), "Critical");

    QCOMPARE(filter->filter(debugMsg), false);
    QCOMPARE(filter->filter(warningMsg), true);
    QCOMPARE(filter->filter(criticalMsg), true);
}

void TestQtloggerHeader::testBasicLogging()
{
    // Create a simple pipeline with stdout sink
    auto pipeline = PipelinePtr::create();
    auto formatter = PatternFormatterPtr::create("%{message}");
    auto sink = StdOutSinkPtr::create();

    // Add formatter and sink to pipeline
    pipeline->append(formatter);
    pipeline->append(sink);

    // Verify pipeline has handlers (use const ref to access public const method)
    const Pipeline &constPipeline = *pipeline;
    QVERIFY(constPipeline.handlers().size() == 2);

    // Create logger and add pipeline
    Logger logger;
    logger << pipeline;

    // Test that SimplePipeline fluent API works
    SimplePipeline sp;
    sp.format("%{message}").sendToStdOut();
    const Pipeline &constSp = sp;
    QVERIFY(constSp.handlers().size() > 0);
}

QTEST_MAIN(TestQtloggerHeader)
#include "test_qtlogger_header.moc"
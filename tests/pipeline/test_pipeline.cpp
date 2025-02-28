// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include <QtTest/QtTest>
#include <QSharedPointer>

#include "qtlogger/pipeline.h"
#include "qtlogger/logmessage.h"
#include "mock_handler.h"

using namespace QtLogger;

class TestPipeline : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Constructor tests
    void testDefaultConstructor();
    void testScopedConstructor();
    void testInitializerListConstructor();

    // Basic functionality tests
    void testType();
    void testAppendSingleHandler();
    void testAppendInitializerList();
    void testAppendNullHandler();
    void testRemoveHandler();
    void testClear();
    void testOperatorLeftShift();

    // Processing tests
    void testProcessEmptyPipeline();
    void testProcessSingleHandler();
    void testProcessMultipleHandlers();
    void testProcessHandlerReturningFalse();
    void testProcessWithNullHandlers();
    void testProcessOrder();

    // Scoped pipeline tests
    void testScopedPipelinePreservesOriginalMessage();
    void testScopedPipelinePreservesOriginalAttributes();
    void testNonScopedPipelineAllowsModifications();

    // Operator overloads tests
    void testOperatorLeftShiftWithPointer();
    void testOperatorLeftShiftWithSharedPointer();

private:
    Pipeline *m_pipeline;
    MockHandlerPtr m_mockHandler1;
    MockHandlerPtr m_mockHandler2;
    MockHandlerPtr m_mockHandler3;
};

void TestPipeline::initTestCase()
{
    // Global test setup if needed
}

void TestPipeline::cleanupTestCase()
{
    // Global test cleanup if needed
}

void TestPipeline::init()
{
    m_pipeline = new Pipeline();
    m_mockHandler1 = MockHandlerPtr::create();
    m_mockHandler2 = MockHandlerPtr::create();
    m_mockHandler3 = MockHandlerPtr::create();
}

void TestPipeline::cleanup()
{
    delete m_pipeline;
    m_mockHandler1.reset();
    m_mockHandler2.reset();
    m_mockHandler3.reset();
}

void TestPipeline::testDefaultConstructor()
{
    Pipeline pipeline;
    QCOMPARE(pipeline.type(), Handler::HandlerType::Pipeline);
}

void TestPipeline::testScopedConstructor()
{
    Pipeline scopedPipeline(true);
    Pipeline nonScopedPipeline(false);
    
    QCOMPARE(scopedPipeline.type(), Handler::HandlerType::Pipeline);
    QCOMPARE(nonScopedPipeline.type(), Handler::HandlerType::Pipeline);
}

void TestPipeline::testInitializerListConstructor()
{
    Pipeline pipeline({m_mockHandler1, m_mockHandler2, m_mockHandler3});
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    pipeline.process(msg);
    
    QCOMPARE(m_mockHandler1->processCallCount(), 1);
    QCOMPARE(m_mockHandler2->processCallCount(), 1);
    QCOMPARE(m_mockHandler3->processCallCount(), 1);
}

void TestPipeline::testType()
{
    QCOMPARE(m_pipeline->type(), Handler::HandlerType::Pipeline);
}

void TestPipeline::testAppendSingleHandler()
{
    m_pipeline->append(m_mockHandler1);
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    m_pipeline->process(msg);
    
    QCOMPARE(m_mockHandler1->processCallCount(), 1);
    QCOMPARE(m_mockHandler1->lastMessage(), QString("test message"));
}

void TestPipeline::testAppendInitializerList()
{
    m_pipeline->append({m_mockHandler1, m_mockHandler2});
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    m_pipeline->process(msg);
    
    QCOMPARE(m_mockHandler1->processCallCount(), 1);
    QCOMPARE(m_mockHandler2->processCallCount(), 1);
}

void TestPipeline::testAppendNullHandler()
{
    HandlerPtr nullHandler;
    m_pipeline->append(nullHandler);
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    bool result = m_pipeline->process(msg);
    
    QVERIFY(result); // Should not crash and return true
}

void TestPipeline::testRemoveHandler()
{
    m_pipeline->append(m_mockHandler1);
    m_pipeline->append(m_mockHandler2);
    
    m_pipeline->remove(m_mockHandler1);
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    m_pipeline->process(msg);
    
    QCOMPARE(m_mockHandler1->processCallCount(), 0);
    QCOMPARE(m_mockHandler2->processCallCount(), 1);
}

void TestPipeline::testClear()
{
    m_pipeline->append(m_mockHandler1);
    m_pipeline->append(m_mockHandler2);
    
    m_pipeline->clear();
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    m_pipeline->process(msg);
    
    QCOMPARE(m_mockHandler1->processCallCount(), 0);
    QCOMPARE(m_mockHandler2->processCallCount(), 0);
}

void TestPipeline::testOperatorLeftShift()
{
    *m_pipeline << m_mockHandler1 << m_mockHandler2;
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    m_pipeline->process(msg);
    
    QCOMPARE(m_mockHandler1->processCallCount(), 1);
    QCOMPARE(m_mockHandler2->processCallCount(), 1);
}

void TestPipeline::testProcessEmptyPipeline()
{
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    bool result = m_pipeline->process(msg);
    
    QVERIFY(result);
}

void TestPipeline::testProcessSingleHandler()
{
    m_pipeline->append(m_mockHandler1);
    
    LogMessage msg(QtWarningMsg, QMessageLogContext(), "warning message");
    bool result = m_pipeline->process(msg);
    
    QVERIFY(result);
    QCOMPARE(m_mockHandler1->processCallCount(), 1);
    QCOMPARE(m_mockHandler1->lastMessage(), QString("warning message"));
    QCOMPARE(m_mockHandler1->lastType(), QtWarningMsg);
}

void TestPipeline::testProcessMultipleHandlers()
{
    m_pipeline->append(m_mockHandler1);
    m_pipeline->append(m_mockHandler2);
    m_pipeline->append(m_mockHandler3);
    
    LogMessage msg(QtCriticalMsg, QMessageLogContext(), "critical message");
    bool result = m_pipeline->process(msg);
    
    QVERIFY(result);
    QCOMPARE(m_mockHandler1->processCallCount(), 1);
    QCOMPARE(m_mockHandler2->processCallCount(), 1);
    QCOMPARE(m_mockHandler3->processCallCount(), 1);
    
    QCOMPARE(m_mockHandler1->lastMessage(), QString("critical message"));
    QCOMPARE(m_mockHandler2->lastMessage(), QString("critical message"));
    QCOMPARE(m_mockHandler3->lastMessage(), QString("critical message"));
}

void TestPipeline::testProcessHandlerReturningFalse()
{
    m_mockHandler1->setReturnValue(true);
    m_mockHandler2->setReturnValue(false); // This should stop processing
    m_mockHandler3->setReturnValue(true);
    
    m_pipeline->append(m_mockHandler1);
    m_pipeline->append(m_mockHandler2);
    m_pipeline->append(m_mockHandler3);
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    bool result = m_pipeline->process(msg);
    
    QVERIFY(result); // Pipeline always returns true
    QCOMPARE(m_mockHandler1->processCallCount(), 1);
    QCOMPARE(m_mockHandler2->processCallCount(), 1);
    QCOMPARE(m_mockHandler3->processCallCount(), 0); // Should not be called
}

void TestPipeline::testProcessWithNullHandlers()
{
    m_pipeline->append(m_mockHandler1);
    m_pipeline->append(HandlerPtr()); // null handler
    m_pipeline->append(m_mockHandler2);
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    bool result = m_pipeline->process(msg);
    
    QVERIFY(result);
    QCOMPARE(m_mockHandler1->processCallCount(), 1);
    QCOMPARE(m_mockHandler2->processCallCount(), 1);
}

void TestPipeline::testProcessOrder()
{
    m_pipeline->append(m_mockHandler1);
    m_pipeline->append(m_mockHandler2);
    m_pipeline->append(m_mockHandler3);
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    m_pipeline->process(msg);
    
    // All handlers should receive the same message
    QCOMPARE(m_mockHandler1->processedMessages(), QStringList() << "test message");
    QCOMPARE(m_mockHandler2->processedMessages(), QStringList() << "test message");
    QCOMPARE(m_mockHandler3->processedMessages(), QStringList() << "test message");
}

void TestPipeline::testScopedPipelinePreservesOriginalMessage()
{
    Pipeline scopedPipeline(true);
    
    m_mockHandler1->setMessageModification("modified message");
    scopedPipeline.append(m_mockHandler1);
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "original message");
    msg.setFormattedMessage("original formatted");
    
    scopedPipeline.process(msg);
    
    // Original formatted message should be restored
    QCOMPARE(msg.formattedMessage(), QString("original formatted"));
}

void TestPipeline::testScopedPipelinePreservesOriginalAttributes()
{
    Pipeline scopedPipeline(true);
    
    m_mockHandler1->setAttributeModification("test_key", "test_value");
    scopedPipeline.append(m_mockHandler1);
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    msg.setAttribute("original_key", "original_value");
    
    QVariantHash originalAttrs = msg.attributes();
    scopedPipeline.process(msg);
    
    // Original attributes should be restored
    QCOMPARE(msg.attributes(), originalAttrs);
    QVERIFY(msg.hasAttribute("original_key"));
    QVERIFY(!msg.hasAttribute("test_key"));
}

void TestPipeline::testNonScopedPipelineAllowsModifications()
{
    Pipeline nonScopedPipeline(false);
    
    m_mockHandler1->setMessageModification("modified message");
    m_mockHandler1->setAttributeModification("test_key", "test_value");
    nonScopedPipeline.append(m_mockHandler1);
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "original message");
    msg.setFormattedMessage("original formatted");
    msg.setAttribute("original_key", "original_value");
    
    nonScopedPipeline.process(msg);
    
    // Modifications should persist
    QCOMPARE(msg.formattedMessage(), QString("modified message"));
    QVERIFY(msg.hasAttribute("test_key"));
    QCOMPARE(msg.attribute("test_key").toString(), QString("test_value"));
}

void TestPipeline::testOperatorLeftShiftWithPointer()
{
    Pipeline *pipeline = new Pipeline();
    *pipeline << m_mockHandler1;
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    pipeline->process(msg);
    
    QCOMPARE(m_mockHandler1->processCallCount(), 1);
    
    delete pipeline;
}

void TestPipeline::testOperatorLeftShiftWithSharedPointer()
{
    PipelinePtr pipeline = PipelinePtr::create();
    *pipeline << m_mockHandler1;
    
    LogMessage msg(QtDebugMsg, QMessageLogContext(), "test message");
    pipeline->process(msg);
    
    QCOMPARE(m_mockHandler1->processCallCount(), 1);
}

QTEST_MAIN(TestPipeline)
#include "test_pipeline.moc"
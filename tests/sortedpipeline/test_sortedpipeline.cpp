#include <QtTest>
#include <QObject>
#include <QSharedPointer>
#include <QStringList>

#include "qtlogger/sortedpipeline.h"
#include "qtlogger/logmessage.h"
#include "mocks.h"

using namespace QtLogger;

class TestSortedPipeline : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testConstructor();
    void testAppendFilter();
    void testSetFormatter();
    void testAppendSink();
    void testAppendPipeline();

    // Handler order tests - main requirement
    void testHandlerExecutionOrder();
    void testHandlerExecutionOrderWithMultipleTypes();
    void testHandlerExecutionOrderComplex();

    // Insert operations tests
    void testInsertBefore();
    void testInsertAfter();
    void testInsertBetween();

    // Clear operations tests
    void testClearFilters();
    void testClearFormatters();
    void testClearSinks();
    void testClearPipelines();
    void testClearType();

    // Edge cases tests
    void testMultipleFormatters();
    void testEmptyPipeline();
    void testNullHandlers();

    // Integration tests
    void testCompleteProcessingChain();
    void testFilterBlocking();
    void testFormatterChaining();

private:
    SortedPipeline *m_pipeline;
    OrderTracker *m_tracker;
    LogMessage createTestMessage(const QString &message = "test message");
};

void TestSortedPipeline::initTestCase()
{
    // Global setup
}

void TestSortedPipeline::cleanupTestCase()
{
    // Global cleanup
}

void TestSortedPipeline::init()
{
    m_pipeline = new SortedPipeline();
    m_tracker = new OrderTracker();
}

void TestSortedPipeline::cleanup()
{
    delete m_pipeline;
    delete m_tracker;
}

LogMessage TestSortedPipeline::createTestMessage(const QString &message)
{
    QMessageLogContext context("test.cpp", 123, "testFunction", "test.category");
    return LogMessage(QtDebugMsg, context, message);
}

void TestSortedPipeline::testConstructor()
{
    QVERIFY(m_pipeline != nullptr);
    QCOMPARE(m_pipeline->type(), Handler::HandlerType::Pipeline);
}

void TestSortedPipeline::testAppendFilter()
{
    auto filter1 = MockFilterPtr::create(true, "filter1");
    auto filter2 = MockFilterPtr::create(true, "filter2");

    m_pipeline->appendFilter(filter1);
    m_pipeline->appendFilter(filter2);

    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(filter1->callCount(), 1);
    QCOMPARE(filter2->callCount(), 1);
}

void TestSortedPipeline::testSetFormatter()
{
    auto formatter1 = MockFormatterPtr::create("format1", "formatter1");
    auto formatter2 = MockFormatterPtr::create("format2", "formatter2");

    m_pipeline->setFormatter(formatter1);
    QCOMPARE(formatter1->callCount(), 0);

    // Setting second formatter should replace the first
    m_pipeline->setFormatter(formatter2);

    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(formatter1->callCount(), 0); // Should not be called
    QCOMPARE(formatter2->callCount(), 1); // Should be called
    QCOMPARE(message.formattedMessage(), QString("format2"));
}

void TestSortedPipeline::testAppendSink()
{
    auto sink1 = MockSinkPtr::create("sink1");
    auto sink2 = MockSinkPtr::create("sink2");

    m_pipeline->appendSink(sink1);
    m_pipeline->appendSink(sink2);

    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(sink1->callCount(), 1);
    QCOMPARE(sink2->callCount(), 1);
}

void TestSortedPipeline::testAppendPipeline()
{
    auto subPipeline1 = MockPipelinePtr::create("pipeline1");
    auto subPipeline2 = MockPipelinePtr::create("pipeline2");

    m_pipeline->appendPipeline(subPipeline1);
    m_pipeline->appendPipeline(subPipeline2);

    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(subPipeline1->callCount(), 1);
    QCOMPARE(subPipeline2->callCount(), 1);
}

void TestSortedPipeline::testHandlerExecutionOrder()
{
    // Create handlers with tracking
    auto attrHandler = QSharedPointer<TrackingAttrHandler>::create("attr1", "value1", "attr1", m_tracker);
    auto filter = QSharedPointer<TrackingFilter>::create(true, "filter1", m_tracker);
    auto formatter = QSharedPointer<TrackingFormatter>::create("formatted", "formatter1", m_tracker);
    auto sink = QSharedPointer<TrackingSink>::create("sink1", m_tracker);
    auto pipeline = QSharedPointer<TrackingPipeline>::create("pipeline1", m_tracker);

    // Add in random order to test that SortedPipeline maintains correct order
    m_pipeline->appendSink(sink);
    m_pipeline->appendFilter(filter);
    m_pipeline->appendPipeline(pipeline);
    m_pipeline->setFormatter(formatter);
    m_pipeline->insertBefore(Handler::HandlerType::Filter, attrHandler);

    auto message = createTestMessage();
    m_pipeline->process(message);

    QStringList expectedOrder = {
        "AttrHandler:attr1",
        "Filter:filter1", 
        "Formatter:formatter1",
        "Sink:sink1",
        "Pipeline:pipeline1"
    };

    QCOMPARE(m_tracker->order(), expectedOrder);
}

void TestSortedPipeline::testHandlerExecutionOrderWithMultipleTypes()
{
    // Multiple AttrHandlers
    auto attr1 = QSharedPointer<TrackingAttrHandler>::create("attr1", "value1", "attr1", m_tracker);
    auto attr2 = QSharedPointer<TrackingAttrHandler>::create("attr2", "value2", "attr2", m_tracker);
    
    // Multiple Filters
    auto filter1 = QSharedPointer<TrackingFilter>::create(true, "filter1", m_tracker);
    auto filter2 = QSharedPointer<TrackingFilter>::create(true, "filter2", m_tracker);
    
    // One Formatter (should replace any previous)
    auto formatter = QSharedPointer<TrackingFormatter>::create("formatted", "formatter1", m_tracker);
    
    // Multiple Sinks
    auto sink1 = QSharedPointer<TrackingSink>::create("sink1", m_tracker);
    auto sink2 = QSharedPointer<TrackingSink>::create("sink2", m_tracker);
    
    // Multiple Pipelines
    auto pipeline1 = QSharedPointer<TrackingPipeline>::create("pipeline1", m_tracker);
    auto pipeline2 = QSharedPointer<TrackingPipeline>::create("pipeline2", m_tracker);

    // Add handlers carefully to ensure proper order
    // First add some filters
    m_pipeline->appendFilter(filter1);
    m_pipeline->appendFilter(filter2);
    
    // Insert attr handlers before filters
    m_pipeline->insertBefore(Handler::HandlerType::Filter, attr1);
    m_pipeline->insertAfter(Handler::HandlerType::AttrHandler, attr2);
    
    // Set formatter (goes after filters)
    m_pipeline->setFormatter(formatter);
    
    // Add sinks (go after formatter)
    m_pipeline->appendSink(sink1);
    m_pipeline->appendSink(sink2);
    
    // Add pipelines (go after sinks)
    m_pipeline->appendPipeline(pipeline1);
    m_pipeline->appendPipeline(pipeline2);

    auto message = createTestMessage();
    m_pipeline->process(message);

    QStringList expectedOrder = {
        "AttrHandler:attr1",
        "AttrHandler:attr2",
        "Filter:filter1",
        "Filter:filter2",
        "Formatter:formatter1",
        "Sink:sink1",
        "Sink:sink2",
        "Pipeline:pipeline1",
        "Pipeline:pipeline2"
    };

    QCOMPARE(m_tracker->order(), expectedOrder);
}

void TestSortedPipeline::testHandlerExecutionOrderComplex()
{
    // Test the exact sequence specified in requirements:
    // 1. любое количество AttrHandler
    // 2. любое количество Filter  
    // 3. только один Formatter
    // 4. любое количество Sink
    // 5. любое количество Pipeline

    auto attr1 = QSharedPointer<TrackingAttrHandler>::create("user", "john", "attr1", m_tracker);
    auto attr2 = QSharedPointer<TrackingAttrHandler>::create("session", "123", "attr2", m_tracker);
    auto attr3 = QSharedPointer<TrackingAttrHandler>::create("module", "auth", "attr3", m_tracker);

    auto filter1 = QSharedPointer<TrackingFilter>::create(true, "levelFilter", m_tracker);
    auto filter2 = QSharedPointer<TrackingFilter>::create(true, "categoryFilter", m_tracker);

    auto formatter = QSharedPointer<TrackingFormatter>::create("[{level}] {message}", "mainFormatter", m_tracker);

    auto sink1 = QSharedPointer<TrackingSink>::create("console", m_tracker);
    auto sink2 = QSharedPointer<TrackingSink>::create("file", m_tracker);
    auto sink3 = QSharedPointer<TrackingSink>::create("network", m_tracker);

    auto pipeline1 = QSharedPointer<TrackingPipeline>::create("errorPipeline", m_tracker);
    auto pipeline2 = QSharedPointer<TrackingPipeline>::create("auditPipeline", m_tracker);

    // Add handlers systematically to test proper ordering
    // First add filters
    m_pipeline->appendFilter(filter1); // levelFilter
    m_pipeline->appendFilter(filter2); // categoryFilter
    
    // Insert attr handlers before filters
    m_pipeline->insertBefore(Handler::HandlerType::Filter, attr2);
    m_pipeline->insertBefore(Handler::HandlerType::AttrHandler, attr1);
    m_pipeline->insertAfter(Handler::HandlerType::AttrHandler, attr3);
    
    // Set formatter
    m_pipeline->setFormatter(formatter);
    
    // Add sinks in order
    m_pipeline->appendSink(sink1); // console
    m_pipeline->appendSink(sink2); // file  
    m_pipeline->appendSink(sink3); // network
    
    // Add pipelines
    m_pipeline->appendPipeline(pipeline1); // errorPipeline
    m_pipeline->appendPipeline(pipeline2); // auditPipeline

    auto message = createTestMessage("Critical error occurred");
    m_pipeline->process(message);

    QStringList expectedOrder = {
        "AttrHandler:attr1",      // First AttrHandler
        "AttrHandler:attr2",      // Second AttrHandler  
        "AttrHandler:attr3",      // Third AttrHandler
        "Filter:levelFilter",     // First Filter
        "Filter:categoryFilter",  // Second Filter
        "Formatter:mainFormatter", // Single Formatter
        "Sink:console",           // First Sink
        "Sink:file",              // Second Sink
        "Sink:network",           // Third Sink
        "Pipeline:errorPipeline", // First Pipeline
        "Pipeline:auditPipeline"  // Second Pipeline
    };

    QCOMPARE(m_tracker->order(), expectedOrder);
}

void TestSortedPipeline::testInsertBefore()
{
    auto filter1 = MockFilterPtr::create(true, "filter1");
    auto filter2 = MockFilterPtr::create(true, "filter2");
    auto filter3 = MockFilterPtr::create(true, "filter3");

    m_pipeline->appendFilter(filter1);
    m_pipeline->appendFilter(filter2);
    m_pipeline->insertBefore(Handler::HandlerType::Filter, filter3);

    // filter3 should be inserted before all existing filters
    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(filter1->callCount(), 1);
    QCOMPARE(filter2->callCount(), 1);
    QCOMPARE(filter3->callCount(), 1);
}

void TestSortedPipeline::testInsertAfter()
{
    auto filter1 = MockFilterPtr::create(true, "filter1");
    auto filter2 = MockFilterPtr::create(true, "filter2");
    auto filter3 = MockFilterPtr::create(true, "filter3");

    m_pipeline->appendFilter(filter1);
    m_pipeline->appendFilter(filter2);
    m_pipeline->insertAfter(Handler::HandlerType::Filter, filter3);

    // filter3 should be inserted after all existing filters
    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(filter1->callCount(), 1);
    QCOMPARE(filter2->callCount(), 1);
    QCOMPARE(filter3->callCount(), 1);
}

void TestSortedPipeline::testInsertBetween()
{
    auto filter = MockFilterPtr::create(true, "filter");
    auto sink = MockSinkPtr::create("sink");
    auto formatter = MockFormatterPtr::create("formatted", "formatter");

    m_pipeline->appendFilter(filter);
    m_pipeline->appendSink(sink);
    
    // Insert formatter between filter and sink
    m_pipeline->insertBetween(Handler::HandlerType::Filter, Handler::HandlerType::Sink, formatter);

    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(filter->callCount(), 1);
    QCOMPARE(formatter->callCount(), 1);
    QCOMPARE(sink->callCount(), 1);
    QCOMPARE(message.formattedMessage(), QString("formatted"));
}

void TestSortedPipeline::testClearFilters()
{
    auto filter1 = MockFilterPtr::create(true, "filter1");
    auto filter2 = MockFilterPtr::create(true, "filter2");
    auto sink = MockSinkPtr::create("sink");

    m_pipeline->appendFilter(filter1);
    m_pipeline->appendFilter(filter2);
    m_pipeline->appendSink(sink);

    m_pipeline->clearFilters();

    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(filter1->callCount(), 0);
    QCOMPARE(filter2->callCount(), 0);
    QCOMPARE(sink->callCount(), 1); // Sink should still be called
}

void TestSortedPipeline::testClearFormatters()
{
    auto formatter = MockFormatterPtr::create("formatted", "formatter");
    auto sink = MockSinkPtr::create("sink");

    m_pipeline->setFormatter(formatter);
    m_pipeline->appendSink(sink);

    m_pipeline->clearFormatters();

    auto message = createTestMessage("original");
    m_pipeline->process(message);

    QCOMPARE(formatter->callCount(), 0);
    QCOMPARE(sink->callCount(), 1);
    QCOMPARE(message.formattedMessage(), QString("original")); // Should remain unformatted
}

void TestSortedPipeline::testClearSinks()
{
    auto filter = MockFilterPtr::create(true, "filter");
    auto sink1 = MockSinkPtr::create("sink1");
    auto sink2 = MockSinkPtr::create("sink2");

    m_pipeline->appendFilter(filter);
    m_pipeline->appendSink(sink1);
    m_pipeline->appendSink(sink2);

    m_pipeline->clearSinks();

    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(filter->callCount(), 1); // Filter should still be called
    QCOMPARE(sink1->callCount(), 0);
    QCOMPARE(sink2->callCount(), 0);
}

void TestSortedPipeline::testClearPipelines()
{
    auto sink = MockSinkPtr::create("sink");
    auto pipeline1 = MockPipelinePtr::create("pipeline1");
    auto pipeline2 = MockPipelinePtr::create("pipeline2");

    m_pipeline->appendSink(sink);
    m_pipeline->appendPipeline(pipeline1);
    m_pipeline->appendPipeline(pipeline2);

    m_pipeline->clearPipelines();

    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(sink->callCount(), 1); // Sink should still be called
    QCOMPARE(pipeline1->callCount(), 0);
    QCOMPARE(pipeline2->callCount(), 0);
}

void TestSortedPipeline::testClearType()
{
    auto filter = MockFilterPtr::create(true, "filter");
    auto sink = MockSinkPtr::create("sink");

    m_pipeline->appendFilter(filter);
    m_pipeline->appendSink(sink);

    m_pipeline->clear(Handler::HandlerType::Filter);

    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(filter->callCount(), 0);
    QCOMPARE(sink->callCount(), 1); // Sink should still be called
}

void TestSortedPipeline::testMultipleFormatters()
{
    auto formatter1 = MockFormatterPtr::create("format1", "formatter1");
    auto formatter2 = MockFormatterPtr::create("format2", "formatter2");

    m_pipeline->setFormatter(formatter1);
    m_pipeline->setFormatter(formatter2); // Should replace formatter1

    auto message = createTestMessage();
    m_pipeline->process(message);

    QCOMPARE(formatter1->callCount(), 0); // Should not be called
    QCOMPARE(formatter2->callCount(), 1); // Should be called
}

void TestSortedPipeline::testEmptyPipeline()
{
    auto message = createTestMessage();
    bool result = m_pipeline->process(message);

    QVERIFY(result); // Empty pipeline should return true
    QCOMPARE(message.formattedMessage(), QString("test message")); // Should remain unchanged
}

void TestSortedPipeline::testNullHandlers()
{
    FilterPtr nullFilter;
    FormatterPtr nullFormatter;
    SinkPtr nullSink;

    // These should not crash and should be ignored
    m_pipeline->appendFilter(nullFilter);
    m_pipeline->setFormatter(nullFormatter);
    m_pipeline->appendSink(nullSink);

    auto message = createTestMessage();
    bool result = m_pipeline->process(message);

    QVERIFY(result);
}

void TestSortedPipeline::testCompleteProcessingChain()
{
    // Set up a complete processing chain
    auto filter = QSharedPointer<TrackingFilter>::create(true, "filter", m_tracker);
    auto formatter = QSharedPointer<TrackingFormatter>::create("FORMATTED: {message}", "formatter", m_tracker);
    auto sink = QSharedPointer<TrackingSink>::create("sink", m_tracker);
    auto attr = QSharedPointer<TrackingAttrHandler>::create("user", "testuser", "attr", m_tracker);

    // Add in proper order
    m_pipeline->appendFilter(filter);
    m_pipeline->insertBefore(Handler::HandlerType::Filter, attr);
    m_pipeline->setFormatter(formatter);
    m_pipeline->appendSink(sink);

    auto message = createTestMessage("test message");
    bool result = m_pipeline->process(message);

    QVERIFY(result);
    QVERIFY(message.hasAttribute("user"));
    QCOMPARE(message.attribute("user").toString(), QString("testuser"));
    QCOMPARE(message.formattedMessage(), QString("FORMATTED: {message}"));

    QStringList expectedOrder = {
        "AttrHandler:attr",
        "Filter:filter",
        "Formatter:formatter", 
        "Sink:sink"
    };

    QCOMPARE(m_tracker->order(), expectedOrder);
}

void TestSortedPipeline::testFilterBlocking()
{
    auto filter1 = MockFilterPtr::create(true, "filter1");
    auto filter2 = MockFilterPtr::create(false, "filter2"); // This will block
    auto filter3 = MockFilterPtr::create(true, "filter3");
    auto sink = MockSinkPtr::create("sink");

    m_pipeline->appendFilter(filter1);
    m_pipeline->appendFilter(filter2);
    m_pipeline->appendFilter(filter3);
    m_pipeline->appendSink(sink);

    auto message = createTestMessage();
    bool result = m_pipeline->process(message);

    QVERIFY(result); // Pipeline always returns true
    QCOMPARE(filter1->callCount(), 1);
    QCOMPARE(filter2->callCount(), 1);
    QCOMPARE(filter3->callCount(), 0); // Should not be called due to blocking
    QCOMPARE(sink->callCount(), 0); // Should not be called due to blocking
}

void TestSortedPipeline::testFormatterChaining()
{
    // Test that only one formatter is active at a time
    auto formatter1 = MockFormatterPtr::create("format1", "formatter1");
    auto formatter2 = MockFormatterPtr::create("format2", "formatter2");
    auto sink = MockSinkPtr::create("sink");

    m_pipeline->setFormatter(formatter1);
    m_pipeline->setFormatter(formatter2); // Should replace formatter1
    m_pipeline->appendSink(sink);

    auto message = createTestMessage("original");
    m_pipeline->process(message);

    QCOMPARE(formatter1->callCount(), 0);
    QCOMPARE(formatter2->callCount(), 1);
    QCOMPARE(message.formattedMessage(), QString("format2"));
    QCOMPARE(sink->messages().first(), QString("format2"));
}

QTEST_MAIN(TestSortedPipeline)
#include "test_sortedpipeline.moc"

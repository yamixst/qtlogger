#include <QtTest/QtTest>
#include <QVariantHash>
#include <QSharedPointer>
#include <QThread>
#include <QThreadPool>
#include <QFuture>
#include <QtConcurrent>

#include "attrhandlers/seqnumberattr.h"
#include "logmessage.h"

using namespace QtLogger;

class TestSeqNumberAttr : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // Basic functionality tests
    void testAttributes();
    void testAttributesContent();
    void testAttributesTypes();
    void testSequentialIncrement();

    // Process method tests (inherited from AttrHandler)
    void testProcessMethod();
    void testProcessAddsAttributes();
    void testProcessReturnsTrue();
    void testProcessMultipleCalls();

    // Handler type tests
    void testHandlerType();

    // Integration tests
    void testWithLogMessage();
    void testMultipleInstances();
    void testSequenceIndependence();

    // Counter behavior tests
    void testInitialValue();
    void testCounterIncrement();
    void testCounterRange();
    void testLargeNumbers();

    // Thread safety tests
    void testThreadSafety();
    void testConcurrentAccess();

    // Edge cases
    void testAttributeKey();
    void testAttributeValue();
    void testRepeatedCalls();

private:
    SeqNumberAttrPtr m_handler;
};

void TestSeqNumberAttr::init()
{
    m_handler = QSharedPointer<SeqNumberAttr>::create();
}

void TestSeqNumberAttr::cleanup()
{
    m_handler.reset();
}

void TestSeqNumberAttr::testAttributes()
{
    QVERIFY(m_handler);
    
    auto attrs = m_handler->attributes();
    QVERIFY(!attrs.isEmpty());
    
    // Should contain exactly one key
    QCOMPARE(attrs.size(), 1);
    QVERIFY(attrs.contains("seq_number"));
}

void TestSeqNumberAttr::testAttributesContent()
{
    auto attrs = m_handler->attributes();
    
    // Test that value is a number
    QVERIFY(attrs["seq_number"].canConvert<int>());
    
    // First call should return 0
    QCOMPARE(attrs["seq_number"].toInt(), 0);
}

void TestSeqNumberAttr::testAttributesTypes()
{
    auto attrs = m_handler->attributes();
    
    QCOMPARE(attrs["seq_number"].type(), QVariant::Int);
}

void TestSeqNumberAttr::testSequentialIncrement()
{
    // Test multiple calls increment the counter
    auto attrs1 = m_handler->attributes();
    QCOMPARE(attrs1["seq_number"].toInt(), 0);
    
    auto attrs2 = m_handler->attributes();
    QCOMPARE(attrs2["seq_number"].toInt(), 1);
    
    auto attrs3 = m_handler->attributes();
    QCOMPARE(attrs3["seq_number"].toInt(), 2);
    
    auto attrs4 = m_handler->attributes();
    QCOMPARE(attrs4["seq_number"].toInt(), 3);
}

void TestSeqNumberAttr::testProcessMethod()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");
    
    QVERIFY(msg.attributes().isEmpty());
    
    bool result = m_handler->process(msg);
    
    QVERIFY(result);
    QVERIFY(!msg.attributes().isEmpty());
    QVERIFY(msg.hasAttribute("seq_number"));
}

void TestSeqNumberAttr::testProcessAddsAttributes()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");
    
    // Add some existing attributes
    msg.setAttribute("existing", "value");
    QCOMPARE(msg.attributes().size(), 1);
    
    m_handler->process(msg);
    
    // Should now have original + seq_number attribute
    QCOMPARE(msg.attributes().size(), 2);
    QVERIFY(msg.hasAttribute("existing"));
    QVERIFY(msg.hasAttribute("seq_number"));
    
    // Original attribute should be preserved
    QCOMPARE(msg.attribute("existing").toString(), QString("value"));
    
    // Sequence number should be 0 for first call
    QCOMPARE(msg.attribute("seq_number").toInt(), 0);
}

void TestSeqNumberAttr::testProcessReturnsTrue()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    LogMessage msg(QtInfoMsg, context, "Test message");
    
    // process() should always return true for AttrHandler
    QVERIFY(m_handler->process(msg));
}

void TestSeqNumberAttr::testProcessMultipleCalls()
{
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    
    // Test multiple messages get incremental sequence numbers
    LogMessage msg1(QtInfoMsg, context, "Message 1");
    m_handler->process(msg1);
    QCOMPARE(msg1.attribute("seq_number").toInt(), 0);
    
    LogMessage msg2(QtInfoMsg, context, "Message 2");
    m_handler->process(msg2);
    QCOMPARE(msg2.attribute("seq_number").toInt(), 1);
    
    LogMessage msg3(QtInfoMsg, context, "Message 3");
    m_handler->process(msg3);
    QCOMPARE(msg3.attribute("seq_number").toInt(), 2);
}

void TestSeqNumberAttr::testHandlerType()
{
    QCOMPARE(m_handler->type(), HandlerType::AttrHandler);
}

void TestSeqNumberAttr::testWithLogMessage()
{
    QMessageLogContext context("seqnum_test.cpp", 200, "main", "app.main");
    LogMessage msg(QtWarningMsg, context, "Sequence test message");
    
    m_handler->process(msg);
    
    auto allAttrs = msg.allAttributes();
    
    // Should have system attributes + sequence number
    QVERIFY(allAttrs.contains("type"));
    QVERIFY(allAttrs.contains("line"));
    QVERIFY(allAttrs.contains("seq_number"));
    
    // Sequence number should be properly integrated
    QCOMPARE(allAttrs["line"].toInt(), 200);
    QCOMPARE(allAttrs["type"].toString(), QString("warning"));
    QCOMPARE(allAttrs["seq_number"].toInt(), 0);
}

void TestSeqNumberAttr::testMultipleInstances()
{
    auto handler1 = QSharedPointer<SeqNumberAttr>::create();
    auto handler2 = QSharedPointer<SeqNumberAttr>::create();
    
    // Each instance should have its own counter
    auto attrs1a = handler1->attributes();
    auto attrs2a = handler2->attributes();
    
    QCOMPARE(attrs1a["seq_number"].toInt(), 0);
    QCOMPARE(attrs2a["seq_number"].toInt(), 0);
    
    // Incrementing one shouldn't affect the other
    auto attrs1b = handler1->attributes();
    QCOMPARE(attrs1b["seq_number"].toInt(), 1);
    
    auto attrs2b = handler2->attributes();
    QCOMPARE(attrs2b["seq_number"].toInt(), 1);
    
    // Continue with handler1
    auto attrs1c = handler1->attributes();
    QCOMPARE(attrs1c["seq_number"].toInt(), 2);
    
    // handler2 should still be at 2
    auto attrs2c = handler2->attributes();
    QCOMPARE(attrs2c["seq_number"].toInt(), 2);
}

void TestSeqNumberAttr::testSequenceIndependence()
{
    auto handler1 = QSharedPointer<SeqNumberAttr>::create();
    auto handler2 = QSharedPointer<SeqNumberAttr>::create();
    
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    
    // Process multiple messages with different handlers
    LogMessage msg1(QtInfoMsg, context, "Message 1");
    handler1->process(msg1);
    QCOMPARE(msg1.attribute("seq_number").toInt(), 0);
    
    LogMessage msg2(QtInfoMsg, context, "Message 2");
    handler2->process(msg2);
    QCOMPARE(msg2.attribute("seq_number").toInt(), 0);
    
    LogMessage msg3(QtInfoMsg, context, "Message 3");
    handler1->process(msg3);
    QCOMPARE(msg3.attribute("seq_number").toInt(), 1);
    
    LogMessage msg4(QtInfoMsg, context, "Message 4");
    handler2->process(msg4);
    QCOMPARE(msg4.attribute("seq_number").toInt(), 1);
}

void TestSeqNumberAttr::testInitialValue()
{
    // First call should return 0
    auto attrs = m_handler->attributes();
    QCOMPARE(attrs["seq_number"].toInt(), 0);
}

void TestSeqNumberAttr::testCounterIncrement()
{
    // Test that counter increments properly
    for (int i = 0; i < 10; ++i) {
        auto attrs = m_handler->attributes();
        QCOMPARE(attrs["seq_number"].toInt(), i);
    }
}

void TestSeqNumberAttr::testCounterRange()
{
    // Test larger numbers
    for (int i = 0; i < 1000; ++i) {
        auto attrs = m_handler->attributes();
        QCOMPARE(attrs["seq_number"].toInt(), i);
    }
}

void TestSeqNumberAttr::testLargeNumbers()
{
    // Advance counter to large number
    for (int i = 0; i < 100000; ++i) {
        m_handler->attributes();
    }
    
    auto attrs = m_handler->attributes();
    QCOMPARE(attrs["seq_number"].toInt(), 100000);
}

void TestSeqNumberAttr::testThreadSafety()
{
    const int numThreads = 10;
    const int numCallsPerThread = 100;
    QVector<QFuture<QVector<int>>> futures;
    
    // Launch multiple threads that call attributes()
    for (int t = 0; t < numThreads; ++t) {
        auto future = QtConcurrent::run([this, numCallsPerThread]() {
            QVector<int> results;
            for (int i = 0; i < numCallsPerThread; ++i) {
                auto attrs = m_handler->attributes();
                results.append(attrs["seq_number"].toInt());
                // Small delay to increase chance of race conditions
                QThread::msleep(1);
            }
            return results;
        });
        futures.append(future);
    }
    
    // Wait for all threads to complete
    QSet<int> allResults;
    for (auto& future : futures) {
        future.waitForFinished();
        auto results = future.result();
        for (int result : results) {
            QVERIFY(!allResults.contains(result)); // Each number should be unique
            allResults.insert(result);
        }
    }
    
    // We should have exactly numThreads * numCallsPerThread unique numbers
    QCOMPARE(allResults.size(), numThreads * numCallsPerThread);
    
    // Numbers should be in range [0, numThreads * numCallsPerThread - 1]
    for (int result : allResults) {
        QVERIFY(result >= 0);
        QVERIFY(result < numThreads * numCallsPerThread);
    }
}

void TestSeqNumberAttr::testConcurrentAccess()
{
    const int numOperations = 1000;
    QVector<QFuture<void>> futures;
    QVector<LogMessage> messages;
    messages.reserve(numOperations);
    
    // Prepare messages
    QMessageLogContext context("test.cpp", 100, "testFunc", "test.category");
    for (int i = 0; i < numOperations; ++i) {
        messages.append(LogMessage(QtInfoMsg, context, QString("Message %1").arg(i)));
    }
    
    // Process messages concurrently
    for (int i = 0; i < numOperations; ++i) {
        auto future = QtConcurrent::run([this, &messages, i]() {
            m_handler->process(messages[i]);
        });
        futures.append(future);
    }
    
    // Wait for all operations to complete
    for (auto& future : futures) {
        future.waitForFinished();
    }
    
    // Collect all sequence numbers
    QSet<int> seqNumbers;
    for (const auto& msg : messages) {
        int seqNum = msg.attribute("seq_number").toInt();
        QVERIFY(!seqNumbers.contains(seqNum)); // Each should be unique
        seqNumbers.insert(seqNum);
    }
    
    // Should have exactly numOperations unique sequence numbers
    QCOMPARE(seqNumbers.size(), numOperations);
}

void TestSeqNumberAttr::testAttributeKey()
{
    auto attrs = m_handler->attributes();
    auto keys = attrs.keys();
    
    // Should have exactly one key
    QCOMPARE(keys.size(), 1);
    QCOMPARE(keys.first(), QString("seq_number"));
}

void TestSeqNumberAttr::testAttributeValue()
{
    auto attrs = m_handler->attributes();
    
    // Value should be convertible to different number types
    QCOMPARE(attrs["seq_number"].toInt(), 0);
    QCOMPARE(attrs["seq_number"].toLongLong(), 0LL);
    QVERIFY(attrs["seq_number"].canConvert<quint32>());
    QVERIFY(attrs["seq_number"].canConvert<double>());
}

void TestSeqNumberAttr::testRepeatedCalls()
{
    // Test that repeated calls continue to increment
    QVector<int> results;
    
    for (int i = 0; i < 50; ++i) {
        auto attrs = m_handler->attributes();
        int seqNum = attrs["seq_number"].toInt();
        results.append(seqNum);
        
        // Each result should equal the iteration number
        QCOMPARE(seqNum, i);
    }
    
    // Verify the sequence is correct
    for (int i = 0; i < results.size(); ++i) {
        QCOMPARE(results[i], i);
    }
}

QTEST_MAIN(TestSeqNumberAttr)
#include "test_seqnumberattr.moc"
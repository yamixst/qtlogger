#pragma once

#include <QSharedPointer>
#include <QStringList>
#include <QVariantHash>
#include <QRegularExpression>

#include "qtlogger/attrhandler.h"
#include "qtlogger/filter.h"
#include "qtlogger/formatter.h"
#include "qtlogger/sink.h"
#include "qtlogger/pipeline.h"
#include "qtlogger/logmessage.h"

namespace QtLogger {

// Mock AttrHandler for testing
class MockAttrHandler : public AttrHandler
{
public:
    explicit MockAttrHandler(const QString &name, const QVariant &value)
        : m_name(name), m_value(value) {}

    QVariantHash attributes(const LogMessage &lmsg) override
    {
        Q_UNUSED(lmsg)
        QVariantHash attrs;
        attrs[m_name] = m_value;
        return attrs;
    }

    QString name() const { return m_name; }
    QVariant value() const { return m_value; }

private:
    QString m_name;
    QVariant m_value;
};

using MockAttrHandlerPtr = QSharedPointer<MockAttrHandler>;

// Mock Filter for testing
class MockFilter : public Filter
{
public:
    explicit MockFilter(bool result = true, const QString &id = QString())
        : m_result(result), m_id(id) {}

    bool filter(const LogMessage &lmsg) override
    {
        Q_UNUSED(lmsg)
        m_callCount++;
        return m_result;
    }

    bool result() const { return m_result; }
    void setResult(bool result) { m_result = result; }
    
    QString id() const { return m_id; }
    int callCount() const { return m_callCount; }
    void resetCallCount() { m_callCount = 0; }

private:
    bool m_result;
    QString m_id;
    mutable int m_callCount = 0;
};

using MockFilterPtr = QSharedPointer<MockFilter>;

// Mock Formatter for testing
class MockFormatter : public Formatter
{
public:
    explicit MockFormatter(const QString &format = "formatted", const QString &id = QString())
        : m_format(format), m_id(id) {}

    QString format(const LogMessage &lmsg) override
    {
        m_callCount++;
        m_lastMessage = lmsg.message();
        return m_format;
    }

    QString formatString() const { return m_format; }
    void setFormat(const QString &format) { m_format = format; }
    
    QString id() const { return m_id; }
    int callCount() const { return m_callCount; }
    QString lastMessage() const { return m_lastMessage; }
    void resetCallCount() { m_callCount = 0; }

private:
    QString m_format;
    QString m_id;
    mutable int m_callCount = 0;
    mutable QString m_lastMessage;
};

using MockFormatterPtr = QSharedPointer<MockFormatter>;

// Mock Sink for testing
class MockSink : public Sink
{
public:
    explicit MockSink(const QString &id = QString())
        : m_id(id) {}

    void send(const LogMessage &lmsg) override
    {
        m_callCount++;
        m_messages.append(lmsg.formattedMessage());
    }

    bool flush() override
    {
        m_flushCount++;
        return true;
    }

    QString id() const { return m_id; }
    int callCount() const { return m_callCount; }
    int flushCount() const { return m_flushCount; }
    QStringList messages() const { return m_messages; }
    
    void resetCallCount() { m_callCount = 0; }
    void resetFlushCount() { m_flushCount = 0; }
    void clearMessages() { m_messages.clear(); }

private:
    QString m_id;
    mutable int m_callCount = 0;
    mutable int m_flushCount = 0;
    mutable QStringList m_messages;
};

using MockSinkPtr = QSharedPointer<MockSink>;

// Mock Pipeline for testing
class MockPipeline : public Pipeline
{
public:
    explicit MockPipeline(const QString &id = QString(), bool scoped = false)
        : Pipeline(scoped), m_id(id) {}

    bool process(LogMessage &lmsg) override
    {
        m_callCount++;
        m_lastMessage = lmsg.message();
        return Pipeline::process(lmsg);
    }

    QString id() const { return m_id; }
    int callCount() const { return m_callCount; }
    QString lastMessage() const { return m_lastMessage; }
    void resetCallCount() { m_callCount = 0; }

private:
    QString m_id;
    mutable int m_callCount = 0;
    mutable QString m_lastMessage;
};

using MockPipelinePtr = QSharedPointer<MockPipeline>;

// Helper class to track handler execution order
class OrderTracker
{
public:
    void reset() { m_order.clear(); }
    void record(const QString &handlerType, const QString &id) 
    { 
        m_order.append(QString("%1:%2").arg(handlerType, id)); 
    }
    QStringList order() const { return m_order; }
    
private:
    QStringList m_order;
};

// Tracking versions of mock handlers - using composition instead of inheritance
// to avoid the final method issue

class TrackingAttrHandler : public AttrHandler
{
public:
    explicit TrackingAttrHandler(const QString &name, const QVariant &value, 
                                const QString &id, OrderTracker *tracker)
        : m_id(id), m_tracker(tracker), m_mock(name, value) {}

    QVariantHash attributes(const LogMessage &lmsg) override
    {
        if (m_tracker) m_tracker->record("AttrHandler", m_id);
        return m_mock.attributes(lmsg);
    }

    QString id() const { return m_id; }

private:
    QString m_id;
    OrderTracker *m_tracker;
    MockAttrHandler m_mock;
};

class TrackingFilter : public Filter
{
public:
    explicit TrackingFilter(bool result, const QString &id, OrderTracker *tracker)
        : m_id(id), m_tracker(tracker), m_mock(result, id) {}

    bool filter(const LogMessage &lmsg) override
    {
        if (m_tracker) m_tracker->record("Filter", m_id);
        return m_mock.filter(lmsg);
    }

    QString id() const { return m_id; }
    int callCount() const { return m_mock.callCount(); }

private:
    QString m_id;
    OrderTracker *m_tracker;
    MockFilter m_mock;
};

class TrackingFormatter : public Formatter
{
public:
    explicit TrackingFormatter(const QString &format, const QString &id, OrderTracker *tracker)
        : m_id(id), m_tracker(tracker), m_mock(format, id) {}

    QString format(const LogMessage &lmsg) override
    {
        if (m_tracker) m_tracker->record("Formatter", m_id);
        return m_mock.format(lmsg);
    }

    QString id() const { return m_id; }
    int callCount() const { return m_mock.callCount(); }

private:
    QString m_id;
    OrderTracker *m_tracker;
    MockFormatter m_mock;
};

class TrackingSink : public Sink
{
public:
    explicit TrackingSink(const QString &id, OrderTracker *tracker)
        : m_id(id), m_tracker(tracker), m_mock(id) {}

    void send(const LogMessage &lmsg) override
    {
        if (m_tracker) m_tracker->record("Sink", m_id);
        m_mock.send(lmsg);
    }

    bool flush() override
    {
        return m_mock.flush();
    }

    QString id() const { return m_id; }
    int callCount() const { return m_mock.callCount(); }
    int flushCount() const { return m_mock.flushCount(); }

private:
    QString m_id;
    OrderTracker *m_tracker;
    MockSink m_mock;
};

class TrackingPipeline : public Pipeline
{
public:
    explicit TrackingPipeline(const QString &id, OrderTracker *tracker, bool scoped = false)
        : Pipeline(scoped), m_id(id), m_tracker(tracker) {}

    bool process(LogMessage &lmsg) override
    {
        if (m_tracker) m_tracker->record("Pipeline", m_id);
        m_callCount++;
        m_lastMessage = lmsg.message();
        return Pipeline::process(lmsg);
    }

    QString id() const { return m_id; }
    int callCount() const { return m_callCount; }
    QString lastMessage() const { return m_lastMessage; }

private:
    QString m_id;
    OrderTracker *m_tracker;
    mutable int m_callCount = 0;
    mutable QString m_lastMessage;
};

} // namespace QtLogger

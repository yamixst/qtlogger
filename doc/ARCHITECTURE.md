# QtLogger Architecture

This document describes the internal architecture and design patterns of the QtLogger library.

## Table of Contents

- [Overview](#overview)
- [Core Components](#core-components)
- [Message Processing Flow](#message-processing-flow)
- [Handler Hierarchy](#handler-hierarchy)
- [Pipeline Architecture](#pipeline-architecture)
- [Threading Model](#threading-model)
- [Design Patterns](#design-patterns)
- [Extensibility](#extensibility)
- [Memory Management](#memory-management)

---

## Overview

QtLogger is built around a **pipeline architecture** where log messages flow through a series of handlers. Each handler can inspect, modify, filter, or output the message. This design provides maximum flexibility while maintaining simplicity.

### Key Principles

1. **Chain of Responsibility**: Messages pass through a chain of handlers
2. **Single Responsibility**: Each handler has one specific purpose
3. **Open/Closed Principle**: Easy to extend without modifying core code
4. **Type Safety**: Strong typing with handler type system
5. **Zero-Copy Where Possible**: Efficient message passing by reference

---

## Core Components

### LogMessage

The `LogMessage` class is the central data structure that flows through the pipeline.

```
┌─────────────────────────────────────┐
│          LogMessage                 │
├─────────────────────────────────────┤
│ Core Properties:                    │
│  - type (QtMsgType)                 │
│  - message (QString)                │
│  - context (file, line, function)   │
│  - category (const char*)           │
│  - time (QDateTime)                 │
│  - steadyTime (chrono)              │
│  - threadId / qthreadptr            │
├─────────────────────────────────────┤
│ Mutable State:                      │
│  - formattedMessage (QString)       │
│  - attributes (QVariantHash)        │
└─────────────────────────────────────┘
```

**Key Features:**
- Immutable core properties (captured at creation)
- Mutable state for pipeline processing
- Custom attributes system for extensibility
- Copy constructor for safe message duplication

### Handler

Abstract base class for all message handlers.

```cpp
class Handler {
    virtual HandlerType type() const;
    virtual bool process(LogMessage &lmsg) = 0;
};
```

**Handler Types:**
- `Handler` - Generic base type
- `AttrHandler` - Adds attributes to messages
- `Filter` - Filters messages (accept/reject)
- `Formatter` - Formats message text
- `Sink` - Outputs messages to destinations
- `Pipeline` - Contains other handlers

**Return Value:**
- `true` - Continue processing
- `false` - Stop processing (filter rejected message)

---

## Message Processing Flow

### Flow Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      Qt Application                         │
│  qDebug() / qInfo() / qWarning() / qCritical() / qFatal()  │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
         ┌───────────────────────────────┐
         │  qInstallMessageHandler()     │
         │  Qt Message Routing System    │
         └───────────┬───────────────────┘
                     │
                     ▼
         ┌───────────────────────────────┐
         │  Logger::messageHandler()     │
         │  (static entry point)         │
         └───────────┬───────────────────┘
                     │
                     ▼
         ┌───────────────────────────────┐
         │  Logger::processMessage()     │
         │  Creates LogMessage           │
         └───────────┬───────────────────┘
                     │
                     ▼
         ┌───────────────────────────────┐
         │     Pipeline::process()       │
         └───────────┬───────────────────┘
                     │
                     ▼
         ┌───────────────────────────────┐
         │   Handler Chain Processing    │
         └───────────┬───────────────────┘
                     │
      ┌──────────────┼──────────────┐
      │              │              │
      ▼              ▼              ▼
┌──────────┐  ┌──────────┐  ┌──────────┐
│  Attr    │  │ Filter   │  │ Format   │
│ Handlers │→ │ Handlers │→ │ Handlers │
└──────────┘  └──────────┘  └────┬─────┘
                                 │
                                 ▼
                            ┌──────────┐
                            │  Sinks   │
                            └────┬─────┘
                                 │
              ┌──────────────────┼──────────────────┐
              ▼                  ▼                  ▼
         ┌─────────┐        ┌─────────┐       ┌─────────┐
         │ Console │        │  File   │       │ Network │
         └─────────┘        └─────────┘       └─────────┘
```

### Processing Stages

1. **Message Creation**: Qt calls the installed message handler
2. **Logger Entry**: Static handler forwards to active Logger instance
3. **LogMessage Construction**: Capture all context and metadata
4. **Pipeline Processing**: Message flows through handler chain
5. **Attribute Enrichment**: AttrHandlers add custom attributes
6. **Filtering**: Filters decide if message continues
7. **Formatting**: Formatters transform message to output string
8. **Output**: Sinks write to destinations

---

## Handler Hierarchy

### Class Diagram

```
                    ┌─────────────┐
                    │   Handler   │
                    │  (abstract) │
                    └──────┬──────┘
                           │
           ┌───────────────┼───────────────┬──────────────┐
           │               │               │              │
     ┌─────▼──────┐  ┌────▼─────┐  ┌──────▼─────┐  ┌────▼────┐
     │ AttrHandler│  │  Filter  │  │ Formatter  │  │  Sink   │
     └─────┬──────┘  └────┬─────┘  └──────┬─────┘  └────┬────┘
           │              │                │              │
    ┌──────┴──────┐  ┌────┴─────┐    ┌────┴─────┐  ┌────┴─────┐
    │             │  │          │    │          │  │          │
┌───▼────┐   ┌───▼──┐ │     ┌──▼──┐ │    ┌────▼┐ │    ┌─────▼┐
│SeqNum  │   │App   │ │     │Level│ │    │Pretty││    │StdOut│
│Attr    │   │Info  │ │     │Filter│    │Format││    │Sink  │
└────────┘   │Attrs │ │     └─────┘ │    └─────┘│    └──────┘
             └──────┘ │             │           │
                      │        ┌────▼┐     ┌────▼┐
                      │        │Cat  │     │Json │
                      │        │Filter    │Format│
                      │        └─────┘     └─────┘
                 ┌────▼──┐
                 │Dup    │
                 │Filter │
                 └───────┘
```

### Handler Interface Contracts

#### AttrHandler
```cpp
class AttrHandler : public Handler {
    virtual QVariantHash attributes(const LogMessage &lmsg) = 0;
    
    bool process(LogMessage &lmsg) override final {
        lmsg.updateAttributes(attributes(lmsg));
        return true;  // Always continues
    }
};
```
- **Purpose**: Enrich messages with metadata
- **Always returns true**: Never blocks messages
- **Examples**: SeqNumberAttr, AppInfoAttrs, HostInfoAttrs

#### Filter
```cpp
class Filter : public Handler {
    virtual bool filter(const LogMessage &lmsg) = 0;
    
    bool process(LogMessage &lmsg) override final {
        return filter(lmsg);  // Pass/block message
    }
};
```
- **Purpose**: Accept or reject messages
- **Returns true/false**: Controls message flow
- **Examples**: LevelFilter, CategoryFilter, RegExpFilter

#### Formatter
```cpp
class Formatter : public Handler {
    virtual QString format(const LogMessage &lmsg) = 0;
    
    bool process(LogMessage &lmsg) override final {
        lmsg.setFormattedMessage(format(lmsg));
        return true;  // Always continues
    }
};
```
- **Purpose**: Transform message to output string
- **Sets formattedMessage**: Stores result in message
- **Examples**: PatternFormatter, JsonFormatter, PrettyFormatter

#### Sink
```cpp
class Sink : public Handler {
    virtual void send(const LogMessage &lmsg) = 0;
    virtual bool flush() { return true; }
    
    bool process(LogMessage &lmsg) override final {
        send(lmsg);
        return true;  // Always continues
    }
};
```
- **Purpose**: Output messages to destinations
- **Terminal operation**: End of processing chain
- **Examples**: FileSink, StdOutSink, HttpSink

---

## Pipeline Architecture

### Pipeline Structure

The `Pipeline` class is itself a `Handler` that contains other handlers.

```
┌────────────────────────────────────────────────────────┐
│                    Pipeline                            │
│                                                        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐           │
│  │Handler 1 │→ │Handler 2 │→ │Handler 3 │→ ...      │
│  └──────────┘  └──────────┘  └──────────┘           │
│                                                        │
│  process() iterates through handlers                   │
│  Short-circuits if any handler returns false          │
└────────────────────────────────────────────────────────┘
```

### Pipeline Types

#### Basic Pipeline
```cpp
Pipeline pipeline;
pipeline.append(handler1);
pipeline.append(handler2);
pipeline << handler3;  // operator<< also supported
```
- Manual handler management
- Flexible ordering
- Direct control

#### SortedPipeline
```cpp
SortedPipeline pipeline;
pipeline.appendAttrHandler(seqNum);
pipeline.appendFilter(levelFilter);
pipeline.setFormatter(prettyFormatter);
pipeline.appendSink(stdoutSink);
```
- Automatic ordering by handler type
- Order: AttrHandlers → Filters → Formatters → Sinks
- Type-safe method calls

#### SimplePipeline
```cpp
SimplePipeline pipeline;
pipeline
    .addSeqNumber()
    .filterLevel(QtWarningMsg)
    .formatPretty()
    .sendToStdOut();
```
- Fluent API
- High-level convenience methods
- Nested pipeline support via `.pipeline()...end()`

### Nested Pipelines

Pipelines can contain other pipelines, enabling parallel processing:

```
┌─────────────────────────────────────────────────────────┐
│                  Root Pipeline                          │
│                                                         │
│  ┌──────────┐  ┌────────────────────────────────┐     │
│  │SeqNumber │→ │     Sub-Pipeline 1             │     │
│  │Attr      │  │  ┌────────┐  ┌──────────┐      │     │
│  └──────────┘  │  │Filter  │→ │StdErr    │      │     │
│                │  │Warning │  │Sink      │      │     │
│       ┌────────┤  └────────┘  └──────────┘      │     │
│       │        └────────────────────────────────┘     │
│       │                                                │
│       │        ┌────────────────────────────────┐     │
│       └───────→│     Sub-Pipeline 2             │     │
│                │  ┌────────┐  ┌──────────┐      │     │
│                │  │Format  │→ │File      │      │     │
│                │  │JSON    │  │Sink      │      │     │
│                │  └────────┘  └──────────┘      │     │
│                └────────────────────────────────┘     │
└─────────────────────────────────────────────────────────┘
```

**Key Points:**
- Each sub-pipeline receives a copy of the message
- Sub-pipelines process independently
- Changes in one sub-pipeline don't affect others
- Enables different output formats to different destinations

---

## Threading Model

### Thread Safety

```
┌──────────────────────────────────────────────────────┐
│                 Logger (Thread-Safe)                 │
│                                                      │
│  ┌────────────────────────────────────────────┐    │
│  │         QMutex / QRecursiveMutex           │    │
│  │  (protects processMessage() execution)     │    │
│  └────────────────────────────────────────────┘    │
│                                                      │
│  Multiple threads can call qDebug() safely          │
│  Serialized access to pipeline processing           │
└──────────────────────────────────────────────────────┘
```

**Default Behavior:**
- All logging calls are mutex-protected
- Safe to call from any thread
- Sequential processing (one message at a time)

### Asynchronous Logging

```
┌───────────────────────┐          ┌───────────────────────┐
│   Application Thread  │          │   Logging Thread      │
│                       │          │                       │
│  qDebug()             │          │                       │
│      │                │          │                       │
│      ▼                │          │                       │
│  Create LogMessage    │          │                       │
│      │                │          │                       │
│      ▼                │          │                       │
│  Post QEvent  ────────┼─────────→│  Receive QEvent       │
│      │                │          │      │                │
│      ▼                │          │      ▼                │
│  Return immediately   │          │  Process Pipeline     │
│                       │          │      │                │
│                       │          │      ▼                │
│                       │          │  Write to Sinks       │
└───────────────────────┘          └───────────────────────┘
```

**OwnThreadHandler<T>:**
```cpp
auto asyncLogger = OwnThreadHandler<SimplePipeline>::create();
asyncLogger->moveToOwnThread();  // Start dedicated thread
```

**Benefits:**
- Non-blocking logging
- I/O operations don't slow down application
- Better throughput for disk/network sinks

**Implementation:**
- Uses Qt event system (QEvent)
- Worker object in dedicated QThread
- Event-driven message processing

---

## Design Patterns

### Chain of Responsibility
Handlers form a chain where each can process or reject the message.

```cpp
bool Pipeline::process(LogMessage &lmsg) {
    for (auto &handler : m_handlers) {
        if (!handler->process(lmsg)) {
            return false;  // Chain broken
        }
    }
    return true;  // Chain completed
}
```

### Template Method
`AttrHandler`, `Filter`, `Formatter`, and `Sink` define the template, subclasses implement specifics.

```cpp
// Template defined in base class
bool Formatter::process(LogMessage &lmsg) final {
    lmsg.setFormattedMessage(format(lmsg));  // Call virtual method
    return true;
}

// Subclass implements specific behavior
QString PrettyFormatter::format(const LogMessage &lmsg) override {
    // Custom formatting logic
}
```

### Strategy Pattern
Different handlers represent different strategies for processing.

```cpp
// Different formatting strategies
pipeline.setFormatter(PrettyFormatter::instance());    // Strategy 1
pipeline.setFormatter(JsonFormatter::instance());      // Strategy 2
pipeline.setFormatter(PatternFormatter("..."));        // Strategy 3
```

### Builder / Fluent Interface
`SimplePipeline` provides a fluent API for configuration.

```cpp
gQtLogger
    .addSeqNumber()        // Returns *this
    .filterLevel(...)      // Returns *this
    .formatPretty()        // Returns *this
    .sendToStdOut();       // Returns *this
```

### Singleton
`Logger` uses singleton pattern for global access.

```cpp
Logger *Logger::instance() {
    static Logger logger;
    return &logger;
}

#define gQtLogger (*QtLogger::Logger::instance())
```

### Composite
`Pipeline` is both a handler and a container of handlers.

```cpp
class Pipeline : public Handler {
    QList<HandlerPtr> m_handlers;  // Contains handlers
    bool process(LogMessage &) override;  // Is a handler
};
```

---

## Extensibility

### Custom Attribute Handler

```cpp
class CustomAttrHandler : public QtLogger::AttrHandler {
public:
    QVariantHash attributes(const QtLogger::LogMessage &lmsg) override {
        return QVariantHash{
            {"custom_field", computeValue()},
            {"request_id", getRequestId()}
        };
    }
};

// Usage
gQtLogger << CustomAttrHandlerPtr::create();
```

### Custom Filter

```cpp
class CustomFilter : public QtLogger::Filter {
public:
    bool filter(const QtLogger::LogMessage &lmsg) override {
        // Custom filtering logic
        return shouldAccept(lmsg);
    }
};

// Usage
gQtLogger << CustomFilterPtr::create();
```

### Custom Formatter

```cpp
class CustomFormatter : public QtLogger::Formatter {
public:
    QString format(const QtLogger::LogMessage &lmsg) override {
        // Custom formatting logic
        return formatMessage(lmsg);
    }
};

// Usage
gQtLogger << CustomFormatterPtr::create();
```

### Custom Sink

```cpp
class CustomSink : public QtLogger::Sink {
public:
    void send(const QtLogger::LogMessage &lmsg) override {
        // Custom output logic
        writeToDestination(lmsg);
    }
    
    bool flush() override {
        // Custom flush logic
        return flushDestination();
    }
};

// Usage
gQtLogger << CustomSinkPtr::create();
```

---

## Memory Management

### Smart Pointers

QtLogger uses Qt's `QSharedPointer` for automatic memory management:

```cpp
using HandlerPtr = QSharedPointer<Handler>;
using FilterPtr = QSharedPointer<Filter>;
using FormatterPtr = QSharedPointer<Formatter>;
using SinkPtr = QSharedPointer<Sink>;
```

**Benefits:**
- Automatic cleanup
- Shared ownership
- Thread-safe reference counting
- No manual delete required

### Message Copying

```cpp
// LogMessage is copyable
LogMessage copy = original;  // Deep copy of mutable state

// Used when distributing to sub-pipelines
for (auto &subpipeline : subpipelines) {
    LogMessage messageCopy = lmsg;  // Each gets a copy
    subpipeline->process(messageCopy);
}
```

### Handler Lifecycle

```cpp
// Handlers are reference-counted
auto handler = HandlerPtr::create();  // Refcount = 1
pipeline << handler;                   // Refcount = 2
handler.reset();                       // Refcount = 1
pipeline.clear();                      // Refcount = 0, deleted
```

---

## Performance Considerations

### Zero-Copy Message Passing

- Messages passed by reference through pipeline
- No unnecessary copying during processing
- Copies only made for sub-pipelines

### Lazy Formatting

- Formatting only occurs if a formatter is present
- Original message preserved
- `formattedMessage()` falls back to original if not formatted

### Short-Circuit Evaluation

- Pipeline stops processing if filter rejects message
- Avoids unnecessary work
- Efficient filtering at early stages

### Thread Pool Consideration

- Single dedicated thread for async logging
- Avoids thread creation overhead
- Qt event loop for efficient message queuing

---

## Configuration Flow

```
┌──────────────────────────────────────────────────────┐
│          Configuration Sources                       │
└───┬───────────────────┬──────────────────────────┬───┘
    │                   │                          │
    ▼                   ▼                          ▼
┌─────────┐      ┌─────────────┐         ┌────────────┐
│Fluent   │      │QSettings/   │         │Environment │
│API      │      │INI File     │         │Variables   │
└────┬────┘      └──────┬──────┘         └─────┬──────┘
     │                  │                      │
     │                  ▼                      │
     │         ┌────────────────┐              │
     │         │Parse Settings  │              │
     │         │Create Handlers │              │
     │         └───────┬────────┘              │
     │                 │                       │
     └─────────────────┼───────────────────────┘
                       │
                       ▼
              ┌────────────────┐
              │Build Pipeline  │
              └────────┬───────┘
                       │
                       ▼
              ┌────────────────┐
              │Install Message │
              │Handler         │
              └────────────────┘
```

---

## Summary

QtLogger's architecture is designed for:

- **Flexibility**: Easy to add new handlers and customize behavior
- **Simplicity**: Clean interfaces and clear responsibilities
- **Performance**: Efficient message processing with minimal overhead
- **Safety**: Thread-safe by default, optional async processing
- **Extensibility**: Well-defined extension points for custom handlers

The pipeline architecture provides a powerful yet intuitive way to configure complex logging scenarios while maintaining code simplicity and readability.
[QtLogger Docs](../index.md) > [API Reference](index.md) > Pipelines

# Pipelines

Pipelines are containers that hold and process handlers in sequence. This section documents the pipeline classes available in QtLogger.

---

## Table of Contents

- [Pipeline](#pipeline)
- [SortedPipeline](#sortedpipeline)
- [SimplePipeline](#simplepipeline)
- [OwnThreadHandler](#ownthreadhandler)

---

## Pipeline

The base pipeline class that holds an ordered list of handlers.

### Inheritance

```
Handler
└── Pipeline
```

### Description

`Pipeline` is a container that processes handlers sequentially. When a message is processed, each handler is called in order. If any handler returns `false`, processing stops and the message is discarded.

### Constructor

```cpp
explicit Pipeline(bool scoped = false);
Pipeline(std::initializer_list<HandlerPtr> handlers, bool scoped = false);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `scoped` | `bool` | If `true`, the pipeline stops at the first filter that rejects the message. If `false`, nested pipelines are processed independently. |
| `handlers` | `std::initializer_list<HandlerPtr>` | Initial list of handlers |

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `type()` | `HandlerType` | Returns `HandlerType::Pipeline` |
| `append(const HandlerPtr &handler)` | `void` | Add a handler to the end |
| `append(std::initializer_list<HandlerPtr> handlers)` | `void` | Add multiple handlers |
| `remove(const HandlerPtr &handler)` | `void` | Remove a handler |
| `clear()` | `void` | Remove all handlers |
| `process(LogMessage &lmsg)` | `bool` | Process a message through all handlers |
| `handlers() const` | `const QList<HandlerPtr> &` | Get the list of handlers |

### Operators

| Operator | Description |
|----------|-------------|
| `Pipeline &operator<<(const HandlerPtr &handler)` | Add a handler (chainable) |

### Processing Behavior

The `process()` method iterates through handlers:

1. For each handler, call `handler->process(lmsg)`
2. If a handler returns `false`:
   - For scoped pipelines: stop and return `false`
   - For non-scoped pipelines: continue to next handler
3. Return `true` when all handlers have been processed

### Example

```cpp
#include "qtlogger.h"

using namespace QtLogger;

// Create a pipeline manually
Pipeline pipeline;
pipeline.append(LevelFilterPtr::create(QtWarningMsg));
pipeline.append(PatternFormatterPtr::create("%{time} [%{type}] %{message}"));
pipeline.append(StdErrSinkPtr::create());

// Or use initializer list
Pipeline pipeline2({
    LevelFilterPtr::create(QtWarningMsg),
    PatternFormatterPtr::create("%{time} [%{type}] %{message}"),
    StdErrSinkPtr::create()
});

// Or use operator<<
Pipeline pipeline3;
pipeline3 << LevelFilterPtr::create(QtWarningMsg)
          << PatternFormatterPtr::create("%{time} [%{type}] %{message}")
          << StdErrSinkPtr::create();
```

---

## SortedPipeline

A pipeline that automatically organizes handlers by type.

### Inheritance

```
Handler
└── Pipeline
    └── SortedPipeline
```

### Description

`SortedPipeline` maintains handlers in a specific order based on their type:

```
AttrHandlers → Filters → Formatters → Sinks → Pipelines
```

This ensures handlers are always processed in the correct order regardless of insertion order.

### Constructor

```cpp
explicit SortedPipeline(bool scoped = false);
```

### Methods

#### General

| Method | Return Type | Description |
|--------|-------------|-------------|
| `clear()` | `void` | Remove all handlers |
| `clear(HandlerType type)` | `void` | Remove handlers of a specific type |

#### Attribute Handlers

| Method | Return Type | Description |
|--------|-------------|-------------|
| `appendAttrHandler(const AttrHandlerPtr &attrHandler)` | `void` | Add an attribute handler |
| `clearAttrHandlers()` | `void` | Remove all attribute handlers |

#### Filters

| Method | Return Type | Description |
|--------|-------------|-------------|
| `appendFilter(const FilterPtr &filter)` | `void` | Add a filter |
| `clearFilters()` | `void` | Remove all filters |

#### Formatters

| Method | Return Type | Description |
|--------|-------------|-------------|
| `setFormatter(const FormatterPtr &formatter)` | `void` | Set the formatter (replaces existing) |
| `clearFormatters()` | `void` | Remove all formatters |

#### Sinks

| Method | Return Type | Description |
|--------|-------------|-------------|
| `appendSink(const SinkPtr &sink)` | `void` | Add a sink |
| `clearSinks()` | `void` | Remove all sinks |

#### Nested Pipelines

| Method | Return Type | Description |
|--------|-------------|-------------|
| `appendPipeline(const PipelinePtr &pipeline)` | `void` | Add a nested pipeline |
| `clearPipelines()` | `void` | Remove all nested pipelines |

#### Advanced Insertion

| Method | Description |
|--------|-------------|
| `insertBetweenNearLeft(const QSet<HandlerType> &leftType, const QSet<HandlerType> &rightType, const HandlerPtr &handler)` | Insert handler between types, closer to left |
| `insertBetweenNearRight(const QSet<HandlerType> &leftType, const QSet<HandlerType> &rightType, const HandlerPtr &handler)` | Insert handler between types, closer to right |

### Example

```cpp
#include "qtlogger.h"

using namespace QtLogger;

SortedPipeline pipeline;

// Add handlers in any order - they'll be sorted automatically
pipeline.appendSink(StdErrSinkPtr::create());
pipeline.appendFilter(LevelFilterPtr::create(QtWarningMsg));
pipeline.appendAttrHandler(SeqNumberAttrPtr::create());
pipeline.setFormatter(PatternFormatterPtr::create("%{seq_number} %{message}"));

// Processing order will be:
// 1. SeqNumberAttr (AttrHandler)
// 2. LevelFilter (Filter)  
// 3. PatternFormatter (Formatter)
// 4. StdErrSink (Sink)
```

---

## SimplePipeline

A pipeline with a fluent API for easy configuration.

### Inheritance

```
Handler
└── Pipeline
    └── SortedPipeline
        └── SimplePipeline
```

### Description

`SimplePipeline` provides a chainable method interface for building pipelines. It supports nested sub-pipelines and is the primary interface for configuring QtLogger.

### Constructor

```cpp
explicit SimplePipeline(bool scoped = false, SimplePipeline *parent = nullptr);
```

### Fluent API Methods

All methods return `SimplePipeline &` for chaining.

#### Attribute Handlers

| Method | Description |
|--------|-------------|
| `addSeqNumber(const QString &name = "seq_number")` | Add sequential message numbering |
| `addAppInfo()` | Add application info (name, version, PID, paths) |
| `addAppUuid(const QString &name = "app_uuid")` | Add persistent application UUID (stored in QSettings) |
| `addHostInfo()` | Add hostname and IP (requires `QTLOGGER_NETWORK`) |
| `attrHandler(std::function<QVariantHash(const LogMessage &)> func)` | Add custom attribute function |

#### Filters

| Method | Description |
|--------|-------------|
| `filterLevel(QtMsgType minLevel)` | Filter by minimum severity level |
| `filterCategory(const QString &rules)` | Filter by Qt logging category rules |
| `filterDuplicate()` | Suppress consecutive duplicate messages |
| `filter(const QString &regexp)` | Filter by regex pattern on message text |
| `filter(std::function<bool(const LogMessage &)> func)` | Custom filter function |

#### Formatters

| Method | Description |
|--------|-------------|
| `format(const QString &pattern)` | Use pattern-based formatting |
| `format(std::function<QString(const LogMessage &)> func)` | Custom formatter function |
| `formatByQt()` | Use Qt's default message formatting |
| `formatPretty(bool colorize = false, int maxCategoryWidth = 15)` | Human-readable format |
| `formatToJson(bool compact = false)` | JSON output format |

#### Sinks

| Method | Description |
|--------|-------------|
| `sendToStdOut(bool colorize = false)` | Output to stdout |
| `sendToStdErr(bool colorize = false)` | Output to stderr |
| `sendToFile(const QString &fileName, int maxFileSize = 0, int maxFileCount = 0, RotatingFileSink::Options options = None)` | File output with optional rotation |
| `sendToIODevice(const QIODevicePtr &device)` | Output to any QIODevice |
| `sendToSignal(QObject *receiver, const char *method)` | Output via Qt signal |
| `sendToHttp(const QString &url)` | HTTP endpoint (requires `QTLOGGER_NETWORK`) |
| `sendToPlatformStdLog()` | Platform-native log output |
| `sendToSyslog()` | Unix syslog (requires `QTLOGGER_SYSLOG`) |
| `sendToSdJournal()` | systemd journal (requires `QTLOGGER_SDJOURNAL`) |
| `sendToAndroidLog()` | Android logcat (Android only) |
| `sendToOsLog()` | macOS/iOS os_log (Apple only) |
| `sendToWinDebug()` | Windows debug output (Windows only) |

#### Pipeline Control

| Method | Description |
|--------|-------------|
| `pipeline()` | Start a nested sub-pipeline |
| `end()` | End current sub-pipeline, return to parent |
| `handler(std::function<bool(LogMessage &)> func)` | Add custom handler function |
| `flush()` | Flush all sinks in the pipeline |

### Nested Pipelines

Use `pipeline()` and `end()` to create sub-pipelines:

```cpp
gQtLogger
    .addSeqNumber()  // Applied to all messages
    
    .pipeline()      // Sub-pipeline 1
        .filterLevel(QtWarningMsg)
        .formatPretty(true)
        .sendToStdErr()
    .end()
    
    .pipeline()      // Sub-pipeline 2
        .formatToJson()
        .sendToFile("app.log")
    .end();
```

Each sub-pipeline processes messages independently. A message filtered out by one sub-pipeline can still be processed by another.

### Example: Complete Configuration

```cpp
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    gQtLogger
        // Global settings
        .moveToOwnThread()
        .addSeqNumber()
        .addAppInfo()
        
        // Console output for development
        .pipeline()
            .filterLevel(QtInfoMsg)
            .filterDuplicate()
            .formatPretty(true, 20)
            .sendToStdErr()
        .end()
        
        // Full log file
        .pipeline()
            .format("%{time yyyy-MM-dd hh:mm:ss.zzz} [%{type:>8}] "
                    "[%{category}] %{shortfile}:%{line} - %{message}")
            .sendToFile("app.log", 10*1024*1024, 5,
                        QtLogger::RotatingFileSink::RotationDaily
                        | QtLogger::RotatingFileSink::Compression)
        .end()
        
        // Error-only file
        .pipeline()
            .filterLevel(QtCriticalMsg)
            .format("%{time} %{file}:%{line} %{function}\n%{message}")
            .sendToFile("errors.log")
        .end();
    
    gQtLogger.installMessageHandler();
    
    qDebug() << "Application started";
    
    return app.exec();
}
```

---

## OwnThreadHandler

A template wrapper that runs a handler in its own dedicated thread.

### Template

```cpp
template<typename BaseHandler>
class OwnThreadHandler : public BaseHandler
```

`BaseHandler` must inherit from `Handler`.

### Description

`OwnThreadHandler` wraps any handler to process messages asynchronously in a dedicated thread. Messages are queued and processed in order without blocking the calling thread.

The `Logger` class inherits from `OwnThreadHandler<SimplePipeline>`, enabling asynchronous logging via `moveToOwnThread()`.

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `moveToOwnThread()` | `OwnThreadHandler &` | Start the dedicated thread |
| `resetOwnThread()` | `void` | Stop the dedicated thread and process remaining messages |
| `ownThread()` | `QThread *` | Get the dedicated thread (or `nullptr`) |
| `ownThreadIsRunning()` | `bool` | Check if the thread is running |

### Behavior

When `moveToOwnThread()` is called:

1. A new `QThread` is created
2. A worker object is moved to the thread
3. Messages are posted as events to the worker
4. The worker processes messages in the dedicated thread

When the application exits (or `resetOwnThread()` is called):

1. Pending messages are processed
2. The thread is stopped gracefully
3. Resources are cleaned up

### Thread Safety

- `moveToOwnThread()` is thread-safe and can be called from any thread
- Multiple calls to `moveToOwnThread()` are idempotent
- The mutex protects internal state during concurrent access

### Example

```cpp
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Enable async logging
    gQtLogger
        .moveToOwnThread()  // Messages queued, processed in background
        .formatPretty()
        .sendToStdErr()
        .sendToFile("app.log", 10*1024*1024, 5);
    
    gQtLogger.installMessageHandler();
    
    // These calls return immediately
    qDebug() << "Message 1";
    qDebug() << "Message 2";
    qDebug() << "Message 3";
    
    // Messages are processed asynchronously
    
    return app.exec();
}
```

### Wrapping Custom Handlers

You can wrap any handler for async processing:

```cpp
#include "qtlogger.h"

using namespace QtLogger;

// Wrap a pipeline for async processing
OwnThreadHandler<Pipeline> asyncPipeline;
asyncPipeline.append(PatternFormatterPtr::create("%{time} %{message}"));
asyncPipeline.append(FileSinkPtr::create("async.log"));
asyncPipeline.moveToOwnThread();

// Use in the logger
gQtLogger << PipelinePtr(&asyncPipeline, [](Pipeline*){});
```

---

## Navigation

| Previous | Next |
|----------|------|
| [← Core Classes](core.md) | [Sinks →](sinks.md) |
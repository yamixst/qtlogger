[QtLogger Docs](../index.md) > API Reference

# API Reference

This section provides detailed documentation for all QtLogger classes and their APIs.

---

## Table of Contents

### Core Classes

- **[Core](core.md)** — Fundamental classes for logging
  - `Logger` — The main logger class and global instance
  - `LogMessage` — Container for log message data
  - Utility functions

### Pipeline Classes

- **[Pipelines](pipelines.md)** — Message processing pipelines
  - `Pipeline` — Base pipeline class
  - `SortedPipeline` — Auto-sorting pipeline by handler type
  - `SimplePipeline` — Fluent API pipeline

### Handler Classes

- **[Sinks](sinks.md)** — Output destinations
  - `StdOutSink` / `StdErrSink` — Console output
  - `FileSink` / `RotatingFileSink` — File output
  - `HttpSink` — HTTP endpoint
  - `SyslogSink` / `SdJournalSink` — System logs
  - `AndroidLogSink` / `OslogSink` — Mobile platforms
  - `SignalSink` — Qt signals
  - `WinDebugSink` — Windows debug output

- **[Formatters](formatters.md)** — Message formatting
  - `PatternFormatter` — Pattern-based formatting
  - `JsonFormatter` — JSON output
  - `PrettyFormatter` — Human-readable colored output
  - `QtLogMessageFormatter` — Qt default formatting
  - `FunctionFormatter` — Custom function-based formatting

- **[Filters](filters.md)** — Message filtering
  - `LevelFilter` — Filter by severity
  - `CategoryFilter` — Filter by Qt category
  - `RegExpFilter` — Filter by regex
  - `DuplicateFilter` — Suppress duplicates
  - `FunctionFilter` — Custom function-based filtering

- **[Attribute Handlers](attributes.md)** — Message enrichment
  - `SeqNumberAttr` — Sequential numbering
  - `AppInfoAttrs` — Application metadata
  - `HostInfoAttrs` — Host information
  - `FunctionAttrHandler` — Custom attributes

---

## Class Hierarchy

```
Handler (abstract)
├── AttrHandler (abstract)
│   ├── SeqNumberAttr
│   ├── AppInfoAttrs
│   ├── HostInfoAttrs
│   └── FunctionAttrHandler
├── Filter (abstract)
│   ├── LevelFilter
│   ├── CategoryFilter
│   ├── RegExpFilter
│   ├── DuplicateFilter
│   └── FunctionFilter
├── Formatter (abstract)
│   ├── PatternFormatter
│   ├── JsonFormatter
│   ├── PrettyFormatter
│   ├── QtLogMessageFormatter
│   └── FunctionFormatter
├── Sink (abstract)
│   ├── IODeviceSink
│   │   └── FileSink
│   │       └── RotatingFileSink
│   ├── StdOutSink
│   ├── StdErrSink
│   ├── HttpSink
│   ├── SyslogSink
│   ├── SdJournalSink
│   ├── AndroidLogSink
│   ├── OslogSink
│   ├── SignalSink
│   └── WinDebugSink
├── Pipeline
│   └── SortedPipeline
│       └── SimplePipeline
│           └── Logger
└── FunctionHandler
```

---

## Smart Pointer Types

All handler classes have corresponding smart pointer typedefs:

| Class | Pointer Type |
|-------|-------------|
| `Handler` | `HandlerPtr` |
| `AttrHandler` | `AttrHandlerPtr` |
| `Filter` | `FilterPtr` |
| `Formatter` | `FormatterPtr` |
| `Sink` | `SinkPtr` |
| `Pipeline` | `PipelinePtr` |
| `Logger` | (singleton, use `gQtLogger`) |
| `LevelFilter` | `LevelFilterPtr` |
| `PatternFormatter` | `PatternFormatterPtr` |
| `StdErrSink` | `StdErrSinkPtr` |
| ... | ... |

Example usage:

```cpp
auto filter = LevelFilterPtr::create(QtWarningMsg);
auto formatter = PatternFormatterPtr::create("%{time} %{message}");
auto sink = StdErrSinkPtr::create();

gQtLogger << filter << formatter << sink;
```

---

## Handler Interface

All handlers implement the `Handler` interface:

```cpp
class Handler
{
public:
    enum class HandlerType { Handler, AttrHandler, Filter, Formatter, Sink, Pipeline };
    
    virtual ~Handler() = default;
    virtual HandlerType type() const;
    virtual bool process(LogMessage &lmsg) = 0;
};
```

The `process()` method returns:
- `true` — Continue to the next handler
- `false` — Stop processing (message is discarded)

---

## Namespace

All QtLogger classes are in the `QtLogger` namespace:

```cpp
using namespace QtLogger;

// Or use fully qualified names:
QtLogger::Logger *logger = QtLogger::Logger::instance();
QtLogger::PatternFormatterPtr fmt = QtLogger::PatternFormatterPtr::create("...");
```

---

## Navigation

| Previous | Next |
|----------|------|
| [← Configuration](../configuration.md) | [Core Classes →](core.md) |
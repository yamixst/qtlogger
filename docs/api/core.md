[QtLogger Docs](../index.md) > [API Reference](index.md) > Core Classes

# Core Classes

This section documents the fundamental classes that form the foundation of QtLogger.

---

## Table of Contents

- [Logger](#logger)
- [LogMessage](#logmessage)
- [Handler](#handler)
- [Utility Functions](#utility-functions)

---

## Logger

The main logger class that manages the logging pipeline and integrates with Qt's message handling system.

### Inheritance

`Logger` inherits from `OwnThreadHandler<SimplePipeline>` (or `SimplePipeline` when `QTLOGGER_NO_THREAD` is defined).

### Global Instance

```cpp
#define gQtLogger (*QtLogger::Logger::instance())
```

Use the global `gQtLogger` macro to access the singleton instance:

```cpp
gQtLogger.configure();
gQtLogger.installMessageHandler();
```

### Static Methods

| Method | Description |
|--------|-------------|
| `Logger *instance()` | Returns the singleton logger instance |
| `void restorePreviousMessageHandler()` | Restores the message handler that was active before QtLogger |

### Instance Methods

#### Configuration

| Method | Description |
|--------|-------------|
| `void configure(const QString &path = {}, int maxFileSize = 0, int maxFileCount = 0, RotatingFileSink::Options options = None, bool async = true)` | Quick configuration with optional file logging |
| `void configure(const QSettings &settings, const QString &group = "logger")` | Configure from QSettings |
| `void configureFromIniFile(const QString &path, const QString &group = "logger")` | Configure from INI file |

#### Message Handler

| Method | Description |
|--------|-------------|
| `void installMessageHandler()` | Install QtLogger as Qt's message handler |
| `void processMessage(QtMsgType type, const QMessageLogContext &context, const QString &message)` | Manually process a log message |

#### Thread Safety

| Method | Description |
|--------|-------------|
| `void lock() const` | Acquire the logger's mutex |
| `void unlock() const` | Release the logger's mutex |
| `QRecursiveMutex *mutex() const` | Get the underlying mutex |

> **Note**: Thread safety methods are only available when `QTLOGGER_NO_THREAD` is not defined.

### Operator Overloads

| Operator | Description |
|----------|-------------|
| `Logger &operator<<(const HandlerPtr &handler)` | Add a handler to the pipeline |
| `Logger &operator<<(const Pipeline &pipeline)` | Add a pipeline |

### Example

```cpp
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Method 1: Simple configuration
    gQtLogger.configure();
    
    // Method 2: Fluent API
    gQtLogger
        .filterLevel(QtInfoMsg)
        .formatPretty()
        .sendToStdErr();
    gQtLogger.installMessageHandler();
    
    // Method 3: Manual handler addition
    gQtLogger << LevelFilterPtr::create(QtWarningMsg)
              << PatternFormatterPtr::create("%{time} %{message}")
              << StdErrSinkPtr::create();
    gQtLogger.installMessageHandler();
    
    qDebug() << "Hello!";
    
    return app.exec();
}
```

---

## LogMessage

A container class that holds all information about a single log message.

### Constructor

```cpp
LogMessage(QtMsgType type, const QMessageLogContext &context, const QString &message);
```

Creates a new log message. This is typically done automatically by the logger when intercepting Qt log calls.

### Basic Accessors

| Method | Return Type | Description |
|--------|-------------|-------------|
| `type()` | `QtMsgType` | Message severity (Debug, Info, Warning, Critical, Fatal) |
| `message()` | `QString` | The original log message text |
| `context()` | `const QMessageLogContext &` | Qt's message context (file, line, function, category) |

### Context Accessors

| Method | Return Type | Description |
|--------|-------------|-------------|
| `line()` | `int` | Source file line number |
| `file()` | `const char *` | Source file path |
| `function()` | `const char *` | Function name/signature |
| `category()` | `const char *` | Logging category name |

### Timestamp Accessors

| Method | Return Type | Description |
|--------|-------------|-------------|
| `time()` | `QDateTime` | Message timestamp |
| `steadyTime()` | `std::chrono::steady_clock::time_point` | Monotonic timestamp for duration calculations |

### Thread Accessors

| Method | Return Type | Description |
|--------|-------------|-------------|
| `threadId()` | `quint64` | Thread identifier |
| `qthreadptr()` | `quintptr` | QThread pointer value |

> **Note**: Thread accessors return 0 when `QTLOGGER_NO_THREAD` is defined.

### Formatted Message

| Method | Return Type | Description |
|--------|-------------|-------------|
| `formattedMessage()` | `QString` | Returns formatted message if set, otherwise the original message |
| `setFormattedMessage(const QString &)` | `void` | Set the formatted message (called by formatters) |
| `isFormatted()` | `bool` | Check if a formatted message has been set |

### Custom Attributes

| Method | Description |
|--------|-------------|
| `QVariant attribute(const QString &name) const` | Get a custom attribute value |
| `void setAttribute(const QString &name, const QVariant &value)` | Set a custom attribute |
| `void setAttributes(const QVariantHash &attrs)` | Replace all attributes |
| `void updateAttributes(const QVariantHash &attrs)` | Merge attributes (existing values preserved) |
| `void removeAttribute(const QString &name)` | Remove an attribute |
| `bool hasAttribute(const QString &name) const` | Check if attribute exists |
| `QVariantHash attributes() const` | Get all custom attributes |
| `QVariantHash allAttributes() const` | Get all attributes including built-in ones |

### All Attributes

The `allAttributes()` method returns a hash containing:

| Key | Type | Description |
|-----|------|-------------|
| `type` | `QString` | "debug", "info", "warning", "critical", or "fatal" |
| `line` | `int` | Source line number |
| `file` | `QString` | Source file path |
| `function` | `QString` | Function name |
| `category` | `QString` | Logging category |
| `message` | `QString` | Original message text |
| `time` | `QDateTime` | Timestamp |
| `threadId` | `quint64` | Thread ID (if threading enabled) |
| + custom attributes | various | Any attributes added by AttrHandlers |

### Example

```cpp
// Custom handler that processes LogMessage
class MyHandler : public Handler
{
public:
    bool process(LogMessage &lmsg) override
    {
        // Access basic info
        qDebug() << "Type:" << lmsg.type();
        qDebug() << "Message:" << lmsg.message();
        qDebug() << "File:" << lmsg.file() << ":" << lmsg.line();
        qDebug() << "Function:" << lmsg.function();
        qDebug() << "Category:" << lmsg.category();
        qDebug() << "Time:" << lmsg.time();
        qDebug() << "Thread:" << lmsg.threadId();
        
        // Access custom attributes
        if (lmsg.hasAttribute("seq_number")) {
            qDebug() << "Seq:" << lmsg.attribute("seq_number").toInt();
        }
        
        // Modify message
        lmsg.setAttribute("processed_by", "MyHandler");
        
        return true;  // Continue processing
    }
};
```

---

## Handler

The abstract base class for all components in the logging pipeline.

### Handler Types

```cpp
enum class HandlerType {
    Handler,      // Generic handler
    AttrHandler,  // Adds attributes to messages
    Filter,       // Filters messages
    Formatter,    // Formats messages
    Sink,         // Outputs messages
    Pipeline      // Contains other handlers
};
```

### Virtual Methods

| Method | Description |
|--------|-------------|
| `virtual ~Handler() = default` | Virtual destructor |
| `virtual HandlerType type() const` | Returns the handler type (default: `Handler`) |
| `virtual bool process(LogMessage &lmsg) = 0` | Process a message. Return `false` to stop the pipeline. |

### FunctionHandler

A convenience handler that wraps a lambda or function:

```cpp
class FunctionHandler : public Handler
{
public:
    using Function = std::function<bool(LogMessage &)>;
    
    FunctionHandler(Function function);
    bool process(LogMessage &lmsg) override;
};
```

**Example:**

```cpp
gQtLogger.handler([](LogMessage &lmsg) {
    // Custom processing
    if (lmsg.message().contains("secret")) {
        return false;  // Drop message
    }
    return true;  // Continue
});
```

---

## Utility Functions

### Message Type Conversion

```cpp
QString qtMsgTypeToString(QtMsgType type, const QString &defaultVal = "debug");
QtMsgType stringToQtMsgType(const QString &str, QtMsgType defaultVal = QtDebugMsg);
```

**Conversion Table:**

| QtMsgType | String |
|-----------|--------|
| `QtDebugMsg` | `"debug"` |
| `QtInfoMsg` | `"info"` |
| `QtWarningMsg` | `"warning"` |
| `QtCriticalMsg` | `"critical"` |
| `QtFatalMsg` | `"fatal"` |

### Filter Rules

```cpp
void setFilterRules(const QString &rules);
```

Sets global Qt logging filter rules. Format: `[category][.level]=true|false`

**Example:**

```cpp
QtLogger::setFilterRules("*.debug=false\nnetwork.*=true");
```

### Message Pattern

```cpp
QString setMessagePattern(const QString &messagePattern);
QString restorePreviousMessagePattern();
```

Sets/restores Qt's global message pattern for `qFormatLogMessage()`.

**Example:**

```cpp
QtLogger::setMessagePattern("%{time} [%{type}] %{message}");
// ... later ...
QtLogger::restorePreviousMessagePattern();
```

---

## Predefined Message Patterns

QtLogger provides some predefined patterns:

```cpp
namespace QtLogger {
    constexpr char DefaultMessagePattern[] = 
        "%{if-category}%{category}: %{endif}%{message}";
    
    constexpr char PrettyMessagePattern[] = 
        "%{time dd.MM.yyyy hh:mm:ss.zzz} "
        "%{if-debug} %{endif}"
        "%{if-info}I%{endif}"
        "%{if-warning}W%{endif}"
        "%{if-critical}E%{endif}"
        "%{if-fatal}F%{endif} "
        "[%{category}] %{message}";
}
```

---

## Navigation

| Previous | Next |
|----------|------|
| [← API Reference](index.md) | [Pipelines →](pipelines.md) |
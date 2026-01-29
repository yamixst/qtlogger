# QtLogger Features

Comprehensive list of all features and capabilities of the QtLogger library.

## Table of Contents

- [Core Features](#core-features)
- [Message Handling](#message-handling)
- [Filters](#filters)
- [Formatters](#formatters)
- [Sinks (Output Destinations)](#sinks-output-destinations)
- [Attributes](#attributes)
- [Pipeline Architecture](#pipeline-architecture)
- [Configuration](#configuration)
- [Threading](#threading)
- [Platform Support](#platform-support)

---

## Core Features

### Drop-in Replacement
- **No code changes required** - works with existing Qt logging functions
- Compatible with `qDebug()`, `qInfo()`, `qWarning()`, `qCritical()`, `qFatal()`
- Full support for `Q_LOGGING_CATEGORY` and category-based logging
- Single line configuration via `gQtLogger.configure()`

### Qt Compatibility
- Qt 5.9 - Qt 6.x support
- C++17 compatible
- Cross-platform (Linux, Windows, macOS, Android, iOS)
- Multiple compiler support (GCC 7+, Clang 5+, MSVC 2017+)

### Flexible Architecture
- Pipeline-based message processing
- Chainable configuration API
- Multiple parallel pipelines support
- Custom handlers and filters
- Header-only or compiled library options

---

## Message Handling

### LogMessage Class
The `LogMessage` class provides rich metadata for each log message:

#### Built-in Properties
- **type**: Log level (debug, info, warning, critical, fatal)
- **message**: The actual log message text
- **time**: Timestamp as `QDateTime`
- **steadyTime**: Monotonic timestamp for performance measurements
- **line**: Line number in source file
- **file**: Full path to source file
- **function**: Function signature
- **category**: Logging category name
- **threadId**: Thread identifier
- **qthreadptr**: QThread pointer

#### Custom Attributes
- Add arbitrary key-value attributes to log messages
- Access attributes via `attribute(name)` / `setAttribute(name, value)`
- Merge attributes with `updateAttributes()`
- Check attribute existence with `hasAttribute(name)`
- Remove attributes with `removeAttribute(name)`

#### Message Formatting
- Store formatted message separately from original
- `formattedMessage()` - get formatted version or fallback to original
- `setFormattedMessage()` - set custom formatted message
- `isFormatted()` - check if message has been formatted

---

## Filters

Filters control which log messages are processed in the pipeline.

### CategoryFilter
Filter messages based on Qt logging categories and levels:
```cpp
.filterCategory("*.debug=false; MyCategory.info=false")
```
- Uses Qt's standard filter rule syntax
- Supports wildcards (`*`)
- Granular control per category and level
- Format: `[category|*][.level]=true|false`

### RegExpFilter
Filter messages using regular expressions:
```cpp
.filter("^(?!.*password|.*secret).*$")  // Exclude sensitive data
```
- Full regex support via `QRegularExpression`
- Match against message content
- Useful for excluding sensitive information

### LevelFilter
Filter by minimum log level:
```cpp
.filterLevel(QtWarningMsg)  // Only warnings and above
```
- Priority-based filtering
- Levels: Debug < Info < Warning < Critical < Fatal

### DuplicateFilter
Remove duplicate consecutive messages:
```cpp
.filterDuplicate()
```
- Prevents log spam
- Compares message content and metadata

### FunctionFilter
Custom filter using lambda or function:
```cpp
.filter([](const QtLogger::LogMessage &lmsg) {
    return lmsg.category() != "unwanted";
})
```
- Full control over filtering logic
- Access to all message properties

---

## Formatters

Formatters transform log messages into desired output format.

### PatternFormatter
Powerful pattern-based formatting with extensive placeholder support:

#### Basic Placeholders
- `%{message}` - Log message text
- `%{type}` - Log level (debug, info, warning, critical)
- `%{category}` - Logging category name
- `%{threadid}` - Thread ID as string
- `%{qthreadptr}` - QThread object pointer
- `%{file}` - Full source file path
- `%{shortfile}` - Source file name only
- `%{shortfile BASEDIR}` - File name with custom base directory stripped
- `%{line}` - Line number
- `%{function}` - Full function signature with cleanup
- `%{func}` - Short function name without arguments

#### Time Placeholders
- `%{time}` - ISO 8601 format (yyyy-MM-ddTHH:mm:ss.zzz)
- `%{time FORMAT}` - Custom Qt date/time format
- `%{time process}` - Seconds since process start (floating point)
- `%{time boot}` - Seconds since system boot (floating point)

#### Custom Attributes
- `%{ATTR}` - Custom attribute value
- `%{ATTR?}` - Optional attribute (no output if not set)
- `%{ATTR?N}` - Optional, remove N chars before if not set
- `%{ATTR?N,M}` - Optional, remove N chars before and M chars after if not set

#### Fixed-Width Formatting
- `%{PLACEHOLDER:[fill][align][width]}` - Padding only
- `%{PLACEHOLDER:[width!]}` - Truncation only
- `%{PLACEHOLDER:[align][width!]}` - Truncation with direction
- `%{PLACEHOLDER:[fill][align][width!]}` - Truncation AND padding

**Alignment:**
- `<` - Left align / truncate from right
- `>` - Right align / truncate from left
- `^` - Center align

**Examples:**
```
%{type:<10}      - Left align, width 10, pad with spaces
%{type:*>10}     - Right align, width 10, pad with '*'
%{type:^15}      - Center align, width 15
%{type:10!}      - Max 10 chars, truncate from right
%{type:*<10!}    - Truncate/pad to exactly 10, left align
```

#### Conditional Blocks
- `%{if-debug}...%{endif}` - Show only for debug messages
- `%{if-info}...%{endif}` - Show only for info messages
- `%{if-warning}...%{endif}` - Show only for warnings
- `%{if-critical}...%{endif}` - Show only for critical messages

#### Special Characters
- `%%` - Escaped percent sign (literal %)

### PrettyFormatter
Human-readable format with colors and alignment:
- Automatic thread ID tracking with short indices
- Category width alignment
- Timestamp formatting
- Type indicators
- Configurable options:
  - `showThreadId()` / `setShowThreadId()`
  - `maxCategoryWidth()` / `setMaxCategoryWidth()`

### JsonFormatter
Output messages as JSON objects:
```cpp
.formatToJson()
```
- All message attributes included
- Standard JSON format
- Easy integration with log aggregation systems

### QtLogMessageFormatter
Use Qt's default message pattern format:
```cpp
.formatByQt()
```
- Compatible with Qt's `qSetMessagePattern()`
- Uses environment variable `QT_MESSAGE_PATTERN` if set

### FunctionFormatter
Custom formatter using lambda or function:
```cpp
.format([](const QtLogger::LogMessage &lmsg) {
    return QString("[%1] %2").arg(lmsg.type()).arg(lmsg.message());
})
```
- Full control over formatting
- Access to all message properties and attributes

---

## Sinks (Output Destinations)

Sinks define where log messages are sent.

### Console Output

#### StdOutSink
Output to standard output stream:
```cpp
.sendToStdOut()
```

With color output (ANSI escape codes):
```cpp
.sendToStdOut(true)  // Enable colored output
```

Or using the sink directly with color modes:
```cpp
auto sink = StdOutSinkPtr::create(ColorMode::Auto);   // Auto-detect TTY
auto sink = StdOutSinkPtr::create(ColorMode::Always); // Always use colors
auto sink = StdOutSinkPtr::create(ColorMode::Never);  // Never use colors
```

#### ColoredConsole Base Class
Both `StdOutSink` and `StdErrSink` inherit from `ColoredConsole`, which provides:

**Instance methods:**
- `setColorMode(ColorMode mode)` - change color mode at runtime
- `colorMode()` - get current color mode
- `colorsEnabled()` - check if colors are currently enabled

**Static utility methods:**
```cpp
// Colorize a message based on log level
QString colored = ColoredConsole::colorize("message", QtWarningMsg);

// Get ANSI color prefix for a log level
QString prefix = ColoredConsole::colorPrefix(QtCriticalMsg);  // "\033[31m"

// Get ANSI reset code
QString reset = ColoredConsole::colorReset();  // "\033[0m"

// Check if output streams are TTY
bool stdoutTty = ColoredConsole::isStdOutTty();
bool stderrTty = ColoredConsole::isStdErrTty();
```

**Class hierarchy:**
```cpp
class StdOutSink : public Sink, public ColoredConsole { ... };
class StdErrSink : public Sink, public ColoredConsole { ... };
```

Color scheme by log level:
- **Debug**: Gray (`\033[90m`)
- **Info**: Green (`\033[32m`)
- **Warning**: Yellow (`\033[33m`)
- **Critical**: Red (`\033[31m`)
- **Fatal**: Bold bright red (`\033[1;91m`)

#### StdErrSink
Output to standard error stream:
```cpp
.sendToStdErr()
```

With color output:
```cpp
.sendToStdErr(true)  // Enable colored output
```

Supports the same `ColorMode` options as `StdOutSink`.

### File Sinks

#### FileSink
Simple file output:
```cpp
.sendToFile("app.log")
```
- Appends to existing file
- Creates file if doesn't exist
- Flushable for immediate write

#### RotatingFileSink
Automatic log rotation:
```cpp
.sendToFile("app.log", maxFileSize, maxFileCount)
```
- Rotates when file exceeds `maxFileSize` bytes
- Keeps up to `maxFileCount` files
- Automatic file naming: `app.log`, `app.log.1`, `app.log.2`, etc.
- Default: 1MB max size, 3 files

### Platform-Specific Sinks

#### PlatformStdLogSink (AndroidLogSink / OslogSink)
Platform-native logging:
```cpp
.sendToPlatformStdLog()
```
- **Android**: Uses `__android_log_write()` (logcat)
- **macOS/iOS**: Uses `os_log()` system
- **Other platforms**: Falls back to stderr

#### SyslogSink (Unix/Linux)
System log daemon:
```cpp
.sendToSyslog()
```
- Uses POSIX syslog facility
- Configurable syslog identifier
- Priority mapping from Qt log levels

#### SdJournalSink (Linux)
SystemD journal:
```cpp
.sendToSdJournal()
```
- Native systemd integration
- Structured logging support
- Preserves metadata fields

#### WinDebugSink (Windows)
Visual Studio debugger output:
```cpp
.sendToWinDebug()
```
- Uses `OutputDebugStringW()`
- Appears in VS Output window

### Network Sinks

#### HttpSink
Send logs to HTTP endpoint:
```cpp
.sendToHttp(QUrl("https://logs.example.com/api/v1/logs"))
```
- POST requests with log data
- Configurable request format
- Asynchronous sending
- Custom `QNetworkAccessManager` support

### Advanced Sinks

#### SignalSink
Qt signal-based sink:
```cpp
auto signalSink = QtLogger::SignalSinkPtr::create();
connect(signalSink, &QtLogger::SignalSink::message, 
        this, &MyClass::handleLogMessage);
```
- Emit log messages as Qt signals
- Connect to custom handlers
- GUI integration

#### IODeviceSink
Output to any `QIODevice`:
```cpp
.setDevice(QSharedPointer<QIODevice>::create(...))
```
- Flexible device support
- Network sockets, files, buffers, etc.

---

## Attributes

Enrich log messages with additional metadata.

### Built-in Attribute Handlers

#### SeqNumberAttr
Add sequential numbering:
```cpp
.addSeqNumber()               // Default attribute name "seq_number"
.addSeqNumber("my_seq")       // Custom attribute name
```
- Thread-safe counter
- Useful for tracking message order

#### AppInfoAttrs
Add application information:
```cpp
.addAppInfo()
```
Adds attributes:
- `appname` - Application name
- `appversion` - Application version
- `orgname` - Organization name
- `orgdomain` - Organization domain

#### HostInfoAttrs
Add system/host information:
```cpp
.addHostInfo()
```
Adds attributes:
- `hostname` - Computer hostname
- `os` - Operating system name
- `arch` - System architecture

### Custom Attribute Handlers

#### FunctionAttrHandler
Add attributes using lambda:
```cpp
.attrHandler([](const QtLogger::LogMessage &lmsg) {
    return QVariantHash{{"user_id", getUserId()}, {"session", getSessionId()}};
})
```
- Dynamic attribute computation
- Access to message context
- Return `QVariantHash` with attributes

---

## Pipeline Architecture

### Pipeline Class
Core message processing chain:
```cpp
Pipeline pipeline;
pipeline << attrHandler << filter << formatter << sink;
```
- Sequential processing
- Each handler can modify or reject messages
- Type-safe handler management

### SimplePipeline
Fluent API for easy configuration:
```cpp
gQtLogger
    .addSeqNumber()
    .filter(...)
    .format(...)
    .sendToStdOut();
```
- Chainable methods
- Automatic handler ordering
- Multiple pipeline support

### SortedPipeline
Automatically ordered pipeline:
- Attribute handlers first
- Then filters
- Then formatters
- Finally sinks
- Maintains correct processing order

### Nested Pipelines
Create sub-pipelines:
```cpp
gQtLogger
    .pipeline()
        .filter(...)
        .sendToFile(...)
    .end()
    .pipeline()
        .format(...)
        .sendToHttp(...)
    .end();
```
- Parallel processing paths
- Different configurations per output
- Independent filtering and formatting

### Custom Handlers
Implement `Handler` interface:
```cpp
class MyHandler : public QtLogger::Handler {
    bool process(QtLogger::LogMessage &lmsg) override {
        // Custom processing
        return true;  // Continue processing
    }
};
```

---

## Configuration

### Programmatic Configuration

#### One-Line Default Configuration
```cpp
gQtLogger.configure();
```
- Pretty formatter
- Platform-specific output (Android Log, OS Log, or stderr)

#### Simple API
```cpp
gQtLogger
    .addSeqNumber()
    .filterLevel(QtWarningMsg)
    .formatPretty()
    .sendToStdErr()
    .sendToFile("app.log", 1024*1024, 5);
```

#### Advanced API
```cpp
gQtLogger.configure(SinkType::StdOut | SinkType::File, "app.log", maxSize, maxCount, async);
```

### File-Based Configuration

#### INI File Format
```cpp
gQtLogger.configure("config.ini");
```

Configuration file example:
```ini
[logger]
filter_rules = "*.debug=false; MyCategory.info=false"
regexp_filter = "^(?!.*password|.*secret).*$"
message_pattern = "[%{time}] [%{type}] %{category}: %{message}"
stdout = true
stdout_color = true
stderr = false
stderr_color = false
platform_std_log = true
syslog_ident = "myapp"
sdjournal = true
path = "myapp.log"
max_file_size = 1048576
max_file_count = 5
async = true
http_url = "https://logs.example.com/api/v1/logs"
http_msg_format = json
```

#### QSettings-Based
```cpp
QSettings settings("config.ini", QSettings::IniFormat);
gQtLogger.configure(settings, "logger");
```

### Environment Variables
- Respects `QT_MESSAGE_PATTERN` for default Qt formatter
- Standard Qt logging environment variables supported

---

## Threading

### Thread Safety
- **Thread-safe** by default
- Mutex-protected message processing
- Safe from multiple threads simultaneously

### Asynchronous Logging

#### OwnThreadHandler
Move pipeline to dedicated thread:
```cpp
gQtLogger.configure(..., async=true);  // From config file
```

Or programmatically:
```cpp
auto ownThreadLogger = OwnThreadHandler<SimplePipeline>::create();
ownThreadLogger->moveToOwnThread();
```

Benefits:
- Non-blocking logging
- Improved application performance
- Background I/O operations
- Thread can be moved back to main thread

### Thread Information
- Automatic thread ID capture
- QThread pointer tracking
- Thread-safe counter attributes

---

## Platform Support

### Operating Systems
- ✅ Linux
- ✅ Windows
- ✅ macOS
- ✅ iOS
- ✅ Android
- ✅ Other Unix-like systems

### Platform-Specific Features

#### Linux
- Syslog support
- SystemD journal integration
- POSIX features

#### Android
- Android Log (logcat) integration
- Automatic priority mapping

#### macOS / iOS
- Unified Logging System (os_log)
- Native system log integration

#### Windows
- OutputDebugString for VS debugger
- Standard file and console output

### Build Options
- Header-only mode
- Compiled library mode
- Conditional compilation for platform features
- Thread support can be disabled (`QTLOGGER_NO_THREAD`)

---

## Advanced Features

### Message Handler Installation
```cpp
gQtLogger.installMessageHandler();           // Install
gQtLogger.restorePreviousMessageHandler();   // Restore
```
- Integrates with Qt's message handling system
- Preserves previous handler
- Can be installed/uninstalled at runtime

### Flushing
```cpp
gQtLogger.flush();
```
- Force immediate write to all sinks
- Useful before application exit
- Recursive flushing through nested pipelines

### Locking
```cpp
gQtLogger.lock();
// Critical section
gQtLogger.unlock();
```
- Manual lock control for advanced usage
- Access to internal mutex

### Handler Type System
All handlers have types for identification:
- `Handler` - Generic handler
- `AttrHandler` - Attribute handler
- `Filter` - Filter handler
- `Formatter` - Formatter handler
- `Sink` - Sink handler
- `Pipeline` - Pipeline handler

### Scoped Pipelines
- Scoped vs. non-scoped pipeline management
- Automatic cleanup
- Resource management

---

## Utility Functions

### Message Pattern Functions
```cpp
QtLogger::setMessagePattern(pattern);
QtLogger::restorePreviousMessagePattern();
```

### Filter Rules Functions
```cpp
QtLogger::setFilterRules(rules);
```

### Type Conversion
```cpp
QString QtLogger::qtMsgTypeToString(QtMsgType type);
QtMsgType QtLogger::stringToQtMsgType(const QString &str);
```

---

## Integration Examples

### Basic Usage
```cpp
#include "qtlogger.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    gQtLogger.configure();
    
    qDebug() << "Hello QtLogger!";
    return app.exec();
}
```

### Advanced Configuration
```cpp
gQtLogger
    .addAppInfo()
    .addSeqNumber()
    .pipeline()
        .filterLevel(QtWarningMsg)
        .formatPretty()
        .sendToStdErr()
    .end()
    .pipeline()
        .formatToJson()
        .sendToFile("app.json", 10*1024*1024, 10)
    .end()
    .pipeline()
        .filter("error|critical")
        .sendToHttp(QUrl("https://logging.example.com"))
    .end();

gQtLogger.installMessageHandler();
```

### Configuration from File
```cpp
gQtLogger.configure(app.applicationDirPath() + "/logging.ini");
```

---

## Version Information

Current version: Defined by `QTLOGGER_VERSION` macro

Semantic Versioning 2.0.0 is followed:
- MAJOR.MINOR.PATCH format
- Breaking changes increment MAJOR
- New features increment MINOR
- Bug fixes increment PATCH

---

## License

MIT License - See LICENSE file for details

## Status

![Project Status](https://img.shields.io/badge/status-experimental-orange)

This is an experimental project under active development.
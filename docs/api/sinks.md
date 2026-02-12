[QtLogger Docs](../index.md) > [API Reference](index.md) > Sinks

# Sinks

Sinks are handlers that output formatted log messages to various destinations. This section documents all available sink classes.

---

## Table of Contents

- [Sink (Base Class)](#sink-base-class)
- [Console Sinks](#console-sinks)
  - [StdOutSink](#stdoutsink)
  - [StdErrSink](#stderrsink)
  - [ColoredConsole](#coloredconsole)
- [File Sinks](#file-sinks)
  - [IODeviceSink](#iodevicesink)
  - [FileSink](#filesink)
  - [RotatingFileSink](#rotatingfilesink)
- [Network Sinks](#network-sinks)
  - [HttpSink](#httpsink)
- [System Log Sinks](#system-log-sinks)
  - [SyslogSink](#syslogsink)
  - [SdJournalSink](#sdjournalsink)
- [Platform-Specific Sinks](#platform-specific-sinks)
  - [AndroidLogSink](#androidlogsink)
  - [OslogSink](#oslogsink)
  - [WinDebugSink](#windebugsink)
  - [PlatformStdSink](#platformstdsink)
- [Other Sinks](#other-sinks)
  - [SignalSink](#signalsink)

---

## Sink (Base Class)

The abstract base class for all output destinations.

### Inheritance

```
Handler
└── Sink
```

### Description

`Sink` provides the interface for outputting log messages. All sink implementations must override the `send()` method to define how messages are written to their destination.

### Virtual Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `send(const LogMessage &lmsg)` | `void` | **Pure virtual.** Output the message to the destination |
| `flush()` | `bool` | Flush buffered output. Returns `true` on success. Default: `true` |

### Inherited Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `type()` | `HandlerType` | Returns `HandlerType::Sink` |
| `process(LogMessage &lmsg)` | `bool` | Calls `send()` and returns `true` |

### Example: Custom Sink

```cpp
#include "qtlogger.h"

class DatabaseSink : public QtLogger::Sink
{
public:
    DatabaseSink(QSqlDatabase &db) : m_db(db) {}
    
    void send(const QtLogger::LogMessage &lmsg) override
    {
        QSqlQuery query(m_db);
        query.prepare("INSERT INTO logs (time, level, message) VALUES (?, ?, ?)");
        query.addBindValue(lmsg.time());
        query.addBindValue(QtLogger::qtMsgTypeToString(lmsg.type()));
        query.addBindValue(lmsg.formattedMessage());
        query.exec();
    }
    
    bool flush() override
    {
        // Commit transaction if using one
        return true;
    }

private:
    QSqlDatabase &m_db;
};
```

---

## Console Sinks

### StdOutSink

Outputs log messages to standard output (stdout).

#### Inheritance

```
Handler
└── Sink
    └── StdOutSink

ColoredConsole (mixin)
```

#### Constructor

```cpp
explicit StdOutSink(ColorMode colorMode = ColorMode::Never);
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `colorMode` | `ColorMode` | `Never` | Color output mode |

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `send(const LogMessage &lmsg)` | `void` | Write message to stdout |
| `flush()` | `bool` | Flush stdout buffer |
| `isTty()` | `bool` | Check if stdout is a terminal |

See [ColoredConsole](#coloredconsole) for color-related methods.

#### SimplePipeline Method

```cpp
SimplePipeline &sendToStdOut(bool colorize = false);
```

When `colorize` is `true`, uses `ColorMode::Auto`.

#### Example

```cpp
#include "qtlogger.h"

// Using fluent API
gQtLogger
    .formatPretty()
    .sendToStdOut(true);  // Colored output

gQtLogger.installMessageHandler();

// Manual creation
auto sink = QtLogger::StdOutSinkPtr::create(QtLogger::ColorMode::Always);
gQtLogger << sink;
```

---

### StdErrSink

Outputs log messages to standard error (stderr).

#### Inheritance

```
Handler
└── Sink
    └── StdErrSink

ColoredConsole (mixin)
```

#### Constructor

```cpp
explicit StdErrSink(ColorMode colorMode = ColorMode::Never);
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `colorMode` | `ColorMode` | `Never` | Color output mode |

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `send(const LogMessage &lmsg)` | `void` | Write message to stderr |
| `flush()` | `bool` | Flush stderr buffer |
| `isTty()` | `bool` | Check if stderr is a terminal |

#### SimplePipeline Method

```cpp
SimplePipeline &sendToStdErr(bool colorize = false);
```

#### Example

```cpp
#include "qtlogger.h"

gQtLogger
    .filterLevel(QtWarningMsg)
    .formatPretty(true)
    .sendToStdErr(true);

gQtLogger.installMessageHandler();
```

---

### ColoredConsole

A mixin class providing ANSI color support for console sinks.

#### ColorMode Enum

```cpp
enum class ColorMode {
    Auto,   // Enable colors if output is a TTY
    Always, // Always enable ANSI colors
    Never   // Never use colors (default)
};
```

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `setColorMode(ColorMode mode)` | `void` | Set the color mode |
| `colorMode()` | `ColorMode` | Get current color mode |
| `colorsEnabled()` | `bool` | Check if colors are currently enabled |

#### Static Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `colorPrefix(QtMsgType type)` | `QString` | Get ANSI color prefix for message type |
| `colorReset()` | `QString` | Get ANSI reset sequence |
| `colorize(const QString &message, QtMsgType type)` | `QString` | Wrap message with color codes |
| `isStdOutTty()` | `bool` | Check if stdout is a terminal |
| `isStdErrTty()` | `bool` | Check if stderr is a terminal |

#### Color Scheme

| Message Type | Color |
|--------------|-------|
| Debug | Dark gray |
| Info | Green |
| Warning | Orange/Yellow |
| Critical | Bold red |
| Fatal | Dark bold red |

---

## File Sinks

### IODeviceSink

Outputs log messages to any `QIODevice`.

#### Inheritance

```
Handler
└── Sink
    └── IODeviceSink
```

#### Constructor

```cpp
explicit IODeviceSink(const QIODevicePtr &device);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `device` | `QSharedPointer<QIODevice>` | The output device |

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `send(const LogMessage &lmsg)` | `void` | Write message to the device |
| `device()` | `const QIODevicePtr &` | Get the underlying device |
| `setDevice(const QIODevicePtr &device)` | `void` | Set a new device |

#### SimplePipeline Method

```cpp
SimplePipeline &sendToIODevice(const QIODevicePtr &device);
```

#### Example

```cpp
#include "qtlogger.h"
#include <QBuffer>

// Write to a memory buffer
auto buffer = QSharedPointer<QBuffer>::create();
buffer->open(QIODevice::WriteOnly);

gQtLogger
    .format("%{time} %{message}\n")
    .sendToIODevice(buffer);

gQtLogger.installMessageHandler();
```

---

### FileSink

Outputs log messages to a file.

#### Inheritance

```
Handler
└── Sink
    └── IODeviceSink
        └── FileSink
```

#### Constructor

```cpp
explicit FileSink(const QString &path);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `path` | `QString` | File path. Supports time placeholders like `%{time:yyyy-MM-dd}` |

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `send(const LogMessage &lmsg)` | `void` | Write message to file |
| `flush()` | `bool` | Flush file buffer to disk |
| `file()` | `QFile *` | Get the underlying QFile |

#### Time Placeholders in Path

The file path can contain time placeholders that are resolved when the sink is created:

```cpp
FileSink("logs/app-%{time:yyyy-MM-dd}.log")
// Creates: logs/app-2024-01-15.log
```

#### Example

```cpp
#include "qtlogger.h"

// Simple file logging
gQtLogger
    .format("%{time yyyy-MM-dd hh:mm:ss} [%{type}] %{message}\n")
    .sendToFile("app.log");

gQtLogger.installMessageHandler();

// Manual creation
auto sink = QtLogger::FileSinkPtr::create("debug.log");
gQtLogger << sink;
```

---

### RotatingFileSink

Outputs log messages to a file with automatic rotation.

#### Inheritance

```
Handler
└── Sink
    └── IODeviceSink
        └── FileSink
            └── RotatingFileSink
```

#### Description

`RotatingFileSink` extends `FileSink` with automatic log rotation based on file size, time, or application startup. Old log files can optionally be compressed with gzip.

#### Constants

```cpp
constexpr static int DefaultMaxFileSize = 1 * 1024 * 1024;  // 1 MB
constexpr static int DefaultMaxFileCount = 5;
```

#### Options Enum

```cpp
enum Option {
    None = 0x00,
    RotationOnStartup = 0x01,  // Rotate when application starts
    RotationDaily = 0x02,      // Rotate when date changes
    Compression = 0x04         // Compress rotated files with gzip
};

Q_DECLARE_FLAGS(Options, Option)
```

#### Constructor

```cpp
explicit RotatingFileSink(const QString &path,
                          int maxFileSize = DefaultMaxFileSize,
                          int maxFileCount = DefaultMaxFileCount,
                          Options options = Option::None);
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `path` | `QString` | — | Log file path |
| `maxFileSize` | `int` | 1 MB | Maximum file size before rotation |
| `maxFileCount` | `int` | 5 | Number of rotated files to keep (0 = unlimited) |
| `options` | `Options` | `None` | Rotation options flags |

#### Rotation Behavior

**Size-based rotation:**
- Triggered when the current file exceeds `maxFileSize`
- Checked before each write

**Daily rotation (`RotationDaily`):**
- Triggered when the message date differs from the last rotation date
- Creates a new file at midnight

**Startup rotation (`RotationOnStartup`):**
- Triggered once when the first message is written
- Useful to separate logs from different application runs

#### Rotated File Naming

Rotated files follow this naming pattern:

```
<basename>.<date>.<index>.<extension>[.gz]
```

Examples:
- `app.2024-01-15.1.log`
- `app.2024-01-15.2.log.gz` (with compression)
- `app.2024-01-16.1.log`

#### File Cleanup

When `maxFileCount` is exceeded:
1. Oldest rotated files are deleted
2. Only the specified number of rotated files are kept
3. The current (active) log file is not counted

#### SimplePipeline Method

```cpp
SimplePipeline &sendToFile(const QString &fileName,
                           int maxFileSize = 0,
                           int maxFileCount = 0,
                           RotatingFileSink::Options options = RotatingFileSink::None);
```

When `maxFileSize` is 0, defaults to 1 MB.
When `maxFileCount` is 0, defaults to 5.

#### Example

```cpp
#include "qtlogger.h"

using namespace QtLogger;

// Full featured rotating log
gQtLogger
    .format("%{time yyyy-MM-dd hh:mm:ss.zzz} [%{type}] [%{category}] %{message}\n")
    .sendToFile("logs/app.log",
                10 * 1024 * 1024,  // 10 MB
                20,                 // Keep 20 files
                RotatingFileSink::RotationOnStartup
                | RotatingFileSink::RotationDaily
                | RotatingFileSink::Compression);

gQtLogger.installMessageHandler();

// Manual creation
auto sink = RotatingFileSinkPtr::create(
    "app.log",
    5 * 1024 * 1024,
    10,
    RotatingFileSink::Compression
);
gQtLogger << sink;
```

---

## Network Sinks

### HttpSink

Sends log messages to an HTTP endpoint.

> **Note**: Requires `QTLOGGER_NETWORK` to be defined.

#### Inheritance

```
Handler
└── Sink
    └── HttpSink
```

#### Constructor

```cpp
explicit HttpSink(const QUrl &url);
HttpSink(const QUrl &url, const Headers &headers);
```

#### Type Aliases

```cpp
using Headers = QList<QPair<QByteArray, QByteArray>>;
```

#### Constructor Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `url` | `QUrl` | HTTP endpoint URL |
| `headers` | `Headers` | Optional HTTP headers to include in requests |

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `send(const LogMessage &lmsg)` | `void` | POST message to HTTP endpoint |
| `setNetworkAccessManager(QNetworkAccessManager *manager)` | `void` | Set custom network manager |
| `setRequest(const QNetworkRequest &request)` | `void` | Set custom request (for headers, etc.) |
| `setHeaders(const Headers &headers)` | `void` | Set HTTP headers for all requests |

#### Request Format

By default, the formatted message is sent as the POST body with `Content-Type: text/plain`.

For JSON logging, use `formatToJson()` before `sendToHttp()`:

```cpp
gQtLogger
    .formatToJson(true)  // Compact JSON
    .sendToHttp("https://logs.example.com/api/logs");
```

#### SimplePipeline Methods

```cpp
SimplePipeline &sendToHttp(const QString &url);
SimplePipeline &sendToHttp(const QString &url,
                           const QList<QPair<QByteArray, QByteArray>> &headers);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `url` | `QString` | HTTP endpoint URL |
| `headers` | `QList<QPair<QByteArray, QByteArray>>` | Optional HTTP headers |

#### Example

```cpp
#include "qtlogger.h"

// Basic HTTP logging
gQtLogger
    .addAppInfo()
    .addHostInfo()
    .formatToJson()
    .sendToHttp("https://logs.mycompany.com/ingest");

gQtLogger.installMessageHandler();

// With custom headers (fluent API)
QList<QPair<QByteArray, QByteArray>> headers = {
    { "Content-Type", "application/json" },
    { "Authorization", "Bearer your-api-key" },
    { "X-Custom-Header", "custom-value" }
};

gQtLogger
    .formatToJson(true)
    .sendToHttp("https://logs.mycompany.com/ingest", headers);

// With custom headers (manual setup)
auto sink = QtLogger::HttpSinkPtr::create(
    QUrl("https://logs.mycompany.com/ingest"),
    {
        { "Authorization", "Bearer your-api-key" }
    }
);

gQtLogger << sink;

// Sentry integration example
gQtLogger
    .addAppInfo()
    .addSysInfo()
    .addHostInfo()
    .filterLevel(QtWarningMsg)
    .formatToSentry()
    .sendToHttp(QtLogger::sentryUrl(), QtLogger::sentryHeaders());
```

---

## System Log Sinks

### SyslogSink

Sends log messages to Unix syslog.

> **Note**: Requires `QTLOGGER_SYSLOG` to be defined. Linux/Unix only.

#### Inheritance

```
Handler
└── Sink
    └── SyslogSink
```

#### Constructor

```cpp
explicit SyslogSink(const QString &ident,
                    int option = QTLOGGER_SYSLOG_LOG_PID,
                    int facility = QTLOGGER_SYSLOG_LOG_USER);
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `ident` | `QString` | — | Syslog identifier (usually app name) |
| `option` | `int` | `LOG_PID` | Syslog options |
| `facility` | `int` | `LOG_USER` | Syslog facility |

#### Syslog Priority Mapping

| QtMsgType | Syslog Priority |
|-----------|-----------------|
| `QtDebugMsg` | `LOG_DEBUG` |
| `QtInfoMsg` | `LOG_INFO` |
| `QtWarningMsg` | `LOG_WARNING` |
| `QtCriticalMsg` | `LOG_ERR` |
| `QtFatalMsg` | `LOG_CRIT` |

#### SimplePipeline Method

```cpp
SimplePipeline &sendToSyslog();
```

Uses the application name as the syslog identifier.

#### Example

```cpp
#include "qtlogger.h"

gQtLogger
    .format("%{category}: %{message}")
    .sendToSyslog();

gQtLogger.installMessageHandler();

// Manual creation with custom options
auto sink = QtLogger::SyslogSinkPtr::create("myapp", LOG_PID | LOG_CONS, LOG_LOCAL0);
gQtLogger << sink;
```

---

### SdJournalSink

Sends log messages to systemd journal.

> **Note**: Requires `QTLOGGER_SDJOURNAL` to be defined. Linux with systemd only.

#### Inheritance

```
Handler
└── Sink
    └── SdJournalSink
```

#### Constructor

```cpp
SdJournalSink();
```

#### Description

`SdJournalSink` sends structured log entries to the systemd journal, including:
- Message text
- Priority level
- Source file and line (if available)
- Category
- Code context

#### Journal Priority Mapping

| QtMsgType | Journal Priority |
|-----------|------------------|
| `QtDebugMsg` | `LOG_DEBUG` (7) |
| `QtInfoMsg` | `LOG_INFO` (6) |
| `QtWarningMsg` | `LOG_WARNING` (4) |
| `QtCriticalMsg` | `LOG_ERR` (3) |
| `QtFatalMsg` | `LOG_CRIT` (2) |

#### SimplePipeline Method

```cpp
SimplePipeline &sendToSdJournal();
```

#### Example

```cpp
#include "qtlogger.h"

gQtLogger
    .sendToSdJournal();

gQtLogger.installMessageHandler();

// View logs with: journalctl -f
```

---

## Platform-Specific Sinks

### AndroidLogSink

Sends log messages to Android logcat.

> **Note**: Automatically available on Android (`QTLOGGER_ANDROIDLOG` is auto-defined).

#### Inheritance

```
Handler
└── Sink
    └── AndroidLogSink
```

#### Constructor

```cpp
AndroidLogSink();
```

#### Logcat Priority Mapping

| QtMsgType | Logcat Priority |
|-----------|-----------------|
| `QtDebugMsg` | `ANDROID_LOG_DEBUG` |
| `QtInfoMsg` | `ANDROID_LOG_INFO` |
| `QtWarningMsg` | `ANDROID_LOG_WARN` |
| `QtCriticalMsg` | `ANDROID_LOG_ERROR` |
| `QtFatalMsg` | `ANDROID_LOG_FATAL` |

#### SimplePipeline Method

```cpp
SimplePipeline &sendToAndroidLog();
```

#### Example

```cpp
#include "qtlogger.h"

gQtLogger
    .format("[%{category}] %{message}")
    .sendToAndroidLog();

gQtLogger.installMessageHandler();
```

---

### OslogSink

Sends log messages to macOS/iOS unified logging system (os_log).

> **Note**: Automatically available on Apple platforms (`QTLOGGER_OSLOG` is auto-defined).

#### Inheritance

```
Handler
└── Sink
    └── OslogSink
```

#### Constructor

```cpp
OslogSink();
```

#### os_log Type Mapping

| QtMsgType | os_log Type |
|-----------|-------------|
| `QtDebugMsg` | `OS_LOG_TYPE_DEBUG` |
| `QtInfoMsg` | `OS_LOG_TYPE_INFO` |
| `QtWarningMsg` | `OS_LOG_TYPE_DEFAULT` |
| `QtCriticalMsg` | `OS_LOG_TYPE_ERROR` |
| `QtFatalMsg` | `OS_LOG_TYPE_FAULT` |

#### SimplePipeline Method

```cpp
SimplePipeline &sendToOsLog();
```

#### Example

```cpp
#include "qtlogger.h"

gQtLogger
    .format("[%{category}] %{message}")
    .sendToOsLog();

gQtLogger.installMessageHandler();
```

---

### WinDebugSink

Sends log messages to the Windows debugger via `OutputDebugString`.

> **Note**: Only available on Windows (`Q_OS_WIN`).

#### Inheritance

```
Handler
└── Sink
    └── WinDebugSink
```

#### Constructor

```cpp
WinDebugSink();
```

#### Description

Messages sent to `WinDebugSink` appear in:
- Visual Studio Output window
- DebugView (Sysinternals)
- Any debugger attached to the process

#### SimplePipeline Method

```cpp
SimplePipeline &sendToWinDebug();
```

#### Example

```cpp
#include "qtlogger.h"

gQtLogger
    .format("[%{type}] %{message}\n")
    .sendToWinDebug();

gQtLogger.installMessageHandler();
```

---

### PlatformStdSink

A type alias that resolves to the platform-appropriate sink.

#### Definition

```cpp
#if defined(QTLOGGER_ANDROIDLOG)
using PlatformStdSink = AndroidLogSink;
#elif defined(QTLOGGER_OSLOG)
using PlatformStdSink = OslogSink;
#else
using PlatformStdSink = StdErrSink;
#endif
```

#### SimplePipeline Method

```cpp
SimplePipeline &sendToPlatformStdLog();
```

This is useful for cross-platform code that should use the native logging system on each platform.

#### Example

```cpp
#include "qtlogger.h"

// Uses logcat on Android, os_log on Apple, stderr elsewhere
gQtLogger
    .formatPretty()
    .sendToPlatformStdLog();

gQtLogger.installMessageHandler();
```

---

## Other Sinks

### SignalSink

Emits log messages as Qt signals, allowing integration with Qt's signal/slot system.

#### Inheritance

```
Handler
└── Sink
    └── SignalSink

QObject
```

#### Constructor

```cpp
explicit SignalSink(QObject *parent = nullptr);
```

#### Signal

```cpp
void message(const QtLogger::LogMessage &lmsg);
```

Emitted for each log message.

#### SimplePipeline Method

```cpp
SimplePipeline &sendToSignal(QObject *receiver, const char *method);
```

Connects the `message` signal to the specified slot.

#### Example

```cpp
#include "qtlogger.h"
#include <QTextEdit>

class LogViewer : public QWidget
{
    Q_OBJECT
public:
    LogViewer(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_textEdit = new QTextEdit(this);
        m_textEdit->setReadOnly(true);
        
        auto layout = new QVBoxLayout(this);
        layout->addWidget(m_textEdit);
    }

public slots:
    void appendLog(const QtLogger::LogMessage &lmsg)
    {
        QString color;
        switch (lmsg.type()) {
        case QtWarningMsg: color = "orange"; break;
        case QtCriticalMsg: color = "red"; break;
        default: color = "black"; break;
        }
        
        m_textEdit->append(
            QString("<span style='color:%1'>%2</span>")
                .arg(color, lmsg.formattedMessage().toHtmlEscaped())
        );
    }

private:
    QTextEdit *m_textEdit;
};

// In main:
LogViewer viewer;
viewer.show();

gQtLogger
    .formatPretty()
    .sendToSignal(&viewer, SLOT(appendLog(QtLogger::LogMessage)));

gQtLogger.installMessageHandler();
```

---

## Navigation

| Previous | Next |
|----------|------|
| [← Pipelines](pipelines.md) | [Formatters →](formatters.md) |
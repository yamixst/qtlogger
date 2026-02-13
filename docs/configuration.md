[QtLogger Docs](index.md) > Configuration

# Configuration Guide

This guide covers all the ways to configure QtLogger, from simple one-line setups to complex multi-pipeline configurations.

---

## Table of Contents

- [Configuration Methods](#configuration-methods)
- [Simple Configuration](#simple-configuration)
- [Fluent API](#fluent-api)
- [INI File Configuration](#ini-file-configuration)
- [Qt Logging Rules](#qt-logging-rules)
- [Configuration Examples](#configuration-examples)

---

## Configuration Methods

QtLogger offers three main approaches to configuration:

| Method | Best For | Flexibility |
|--------|----------|-------------|
| `configure()` | Quick setup, simple logging | Low |
| Fluent API | Complex pipelines, multiple outputs | High |
| INI File | Runtime configuration, no recompilation | Medium |

---

## Simple Configuration

### Basic Console Output

The simplest way to start:

```cpp
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    gQtLogger.configure();  // Pretty colored output to stderr
    
    qDebug() << "Hello, QtLogger!";
    
    return app.exec();
}
```

This sets up:
- Pretty formatted output with colors
- Output to stderr
- Automatic message handler installation

### File Logging with Rotation

```cpp
gQtLogger.configure("app.log",                // File path
                    1024 * 1024,               // Max size: 1 MB
                    5,                         // Keep 5 rotated files
                    QtLogger::RotatingFileSink::Compression);  // Compress old files
```

### Configure Options

The full signature:

```cpp
void configure(const QString &path = {},
               int maxFileSize = 0,
               int maxFileCount = 0,
               RotatingFileSink::Options options = RotatingFileSink::Option::None,
               bool async = true);
```

| Parameter | Description | Default |
|-----------|-------------|---------|
| `path` | Log file path. Empty = no file logging | Empty |
| `maxFileSize` | Maximum file size before rotation (bytes) | 1 MB |
| `maxFileCount` | Number of rotated files to keep | 5 |
| `options` | Rotation options (see below) | None |
| `async` | Enable asynchronous logging | `true` |

### Rotation Options

`RotatingFileSink::Options` is a flags enum:

| Flag | Description |
|------|-------------|
| `None` | No special rotation behavior |
| `RotationOnStartup` | Rotate existing log file when application starts |
| `RotationDaily` | Rotate log file when date changes |
| `Compression` | Compress rotated files with gzip |

Options can be combined:

```cpp
gQtLogger.configure("app.log", 1024*1024, 5,
                    QtLogger::RotatingFileSink::RotationOnStartup
                    | QtLogger::RotatingFileSink::RotationDaily
                    | QtLogger::RotatingFileSink::Compression);
```

---

## Fluent API

The fluent API provides full control over the logging pipeline through method chaining.

### Basic Structure

```cpp
gQtLogger
    .addSeqNumber()           // Add attribute handlers
    .filterLevel(QtInfoMsg)   // Add filters
    .format("...")            // Set formatter
    .sendToStdErr();          // Add sinks

gQtLogger.installMessageHandler();  // Activate!
```

> **Important**: When using the fluent API, you must call `installMessageHandler()` to activate the logger.

### Available Methods

#### Attribute Handlers

| Method | Description |
|--------|-------------|
| `addSeqNumber(name)` | Add sequential message number. Default name: `"seq_number"` |
| `addAppInfo()` | Add application info (name, version, PID, paths) |
| `addAppUuid(name)` | Add persistent application UUID (stored in QSettings). Default name: `"app_uuid"` |
| `addSysInfo()` | Add system info (OS, kernel, CPU architecture) |
| `addHostInfo()` | Add host info (hostname, IP). Requires `QTLOGGER_NETWORK` |
| `attrHandler(func)` | Add custom attribute function |

#### Filters

| Method | Description |
|--------|-------------|
| `filterLevel(minLevel)` | Only pass messages >= minLevel |
| `filterCategory(rules)` | Filter by Qt logging category rules |
| `filterDuplicate()` | Suppress consecutive duplicate messages |
| `filter(regexp)` | Filter by regex pattern on message text |
| `filter(func)` | Custom filter function |

#### Formatters

| Method | Description |
|--------|-------------|
| `format(pattern)` | Use [PatternFormatter](api/formatters.md#patternformatter) with pattern |
| `format(func)` | Custom formatter function |
| `formatByQt()` | Use Qt's default message formatting |
| `formatPretty(colorize, maxCategoryWidth)` | Human-readable format with optional colors |
| `formatToJson(compact)` | JSON output |

#### Sinks

| Method | Description |
|--------|-------------|
| `sendToStdOut(colorize)` | Output to stdout |
| `sendToStdErr(colorize)` | Output to stderr |
| `sendToFile(path, maxSize, maxCount, options)` | File with optional rotation |
| `sendToIODevice(device)` | Any QIODevice |
| `sendToSignal(receiver, method)` | Qt signal/slot |
| `sendToHttp(url)` | HTTP endpoint. Requires `QTLOGGER_NETWORK` |
| `sendToSyslog()` | Unix syslog. Requires `QTLOGGER_SYSLOG` |
| `sendToSdJournal()` | systemd journal. Requires `QTLOGGER_SDJOURNAL` |
| `sendToPlatformStdLog()` | Platform-native log (logcat, os_log, or stderr) |
| `sendToWinDebug()` | Windows debug output. Windows only |
| `sendToAndroidLog()` | Android logcat. Android only |
| `sendToOsLog()` | macOS/iOS os_log. Apple platforms only |

#### Pipeline Control

| Method | Description |
|--------|-------------|
| `pipeline()` | Start a nested sub-pipeline |
| `end()` | End current sub-pipeline, return to parent |
| `handler(func)` | Add custom handler function |
| `flush()` | Flush all sinks |

#### Async Control

| Method | Description |
|--------|-------------|
| `moveToOwnThread()` | Enable asynchronous logging |

### Example: Multiple Outputs

```cpp
gQtLogger
    .moveToOwnThread()
    .addSeqNumber()
    
    // Sub-pipeline 1: Colored console for warnings+
    .pipeline()
        .filterLevel(QtWarningMsg)
        .formatPretty(true)
        .sendToStdErr()
    .end()
    
    // Sub-pipeline 2: Full log file
    .pipeline()
        .format("%{time yyyy-MM-dd hh:mm:ss.zzz} [%{type}] %{message}")
        .sendToFile("app.log", 10*1024*1024, 10,
                    QtLogger::RotatingFileSink::RotationDaily
                    | QtLogger::RotatingFileSink::Compression)
    .end()
    
    // Sub-pipeline 3: JSON to HTTP
    .pipeline()
        .addAppInfo()
        .addHostInfo()
        .formatToJson()
        .sendToHttp("https://logs.example.com/ingest")
    .end();

gQtLogger.installMessageHandler();
```

---

## INI File Configuration

For runtime configuration without recompilation, use INI files.

### Loading INI Configuration

```cpp
gQtLogger.configureFromIniFile("config.ini");
// or
gQtLogger.configureFromIniFile("/path/to/config.ini", "logger");  // Custom group name
```

You can also load from existing QSettings:

```cpp
QSettings settings("config.ini", QSettings::IniFormat);
gQtLogger.configure(settings, "logger");
```

### INI File Format

All settings go under a `[logger]` group (or custom group name):

```ini
[logger]
;; Enable asynchronous logging
async = true

;; Qt Filter rules
;; Format: [<category>|*][.debug|.info|.warning|.critical]=true|false;...
filter_rules = "*.debug=true"

;; Filter with regular expression
; regexp_filter = "^(?!.*password).*$"

;; Message pattern (see PatternFormatter documentation)
message_pattern = "%{time yyyy-MM-dd hh:mm:ss} %{type:^8} [%{category}] %{message}"

;; Console output
stdout = false
stdout_color = false
stderr = false
stderr_color = false

;; Platform-native output (Android logcat, macOS os_log, or stderr)
platform_std_log = true

;; Unix syslog (Linux/Unix only)
; syslog_ident = "myapp"

;; systemd journal (Linux only)
sdjournal = false

;; File output
path = "app.log"
max_file_size = 1048576
max_file_count = 5
rotate_on_startup = true
rotate_daily = false
compress_old_files = false

;; HTTP output
; http_url = "http://localhost:8080/log"
; http_msg_format = json
```

### INI Settings Reference

#### General Settings

| Key | Type | Description |
|-----|------|-------------|
| `async` | bool | Enable asynchronous logging (`true`/`false`) |
| `filter_rules` | string | Qt logging category filter rules |
| `regexp_filter` | string | Regular expression to filter messages |
| `message_pattern` | string | Format pattern for output |

#### Console Output

| Key | Type | Description |
|-----|------|-------------|
| `stdout` | bool | Enable stdout output |
| `stdout_color` | bool | Enable ANSI colors for stdout |
| `stderr` | bool | Enable stderr output |
| `stderr_color` | bool | Enable ANSI colors for stderr |
| `platform_std_log` | bool | Use platform-native logging |

#### System Logs

| Key | Type | Description |
|-----|------|-------------|
| `syslog_ident` | string | Syslog identifier (enables syslog when set) |
| `sdjournal` | bool | Enable systemd journal output |

#### File Output

| Key | Type | Description |
|-----|------|-------------|
| `path` | string | Log file path |
| `max_file_size` | int | Maximum file size in bytes (default: 1048576 = 1MB) |
| `max_file_count` | int | Number of rotated files to keep (default: 5) |
| `rotate_on_startup` | bool | Rotate existing file on application start |
| `rotate_daily` | bool | Rotate file when date changes |
| `compress_old_files` | bool | Compress rotated files with gzip |

#### Network Output

| Key | Type | Description |
|-----|------|-------------|
| `http_url` | string | HTTP endpoint URL |
| `http_msg_format` | string | Message format: `raw`, `default`, or `json` |

---

## Qt Logging Rules

QtLogger integrates with Qt's standard logging rules system.

### Setting Filter Rules

In code:

```cpp
QtLogger::setFilterRules("*.debug=false\nnetwork.*=true");
```

Or via INI file:

```ini
filter_rules = "*.debug=false;network.*=true"
```

Or via environment variable:

```bash
export QT_LOGGING_RULES="*.debug=false;network.*=true"
```

### Rule Syntax

```
[category][.type]=true|false
```

- `*` matches any category
- `.debug`, `.info`, `.warning`, `.critical` specify the log level
- Multiple rules separated by `;` or newlines

### Examples

| Rule | Effect |
|------|--------|
| `*.debug=false` | Disable all debug messages |
| `network.*=true` | Enable all messages in `network` category |
| `*.warning=true` | Enable all warnings |
| `app.ui.debug=false` | Disable debug for `app.ui` category |

### Using with QtLogger Filters

QtLogger's category filter provides similar functionality:

```cpp
gQtLogger
    .filterCategory("*.debug=false;network.*=true")
    .formatPretty()
    .sendToStdErr();
```

The difference:
- Qt filter rules affect the global Qt logging system
- `filterCategory()` only affects that specific pipeline

---

## Configuration Examples

### Development Setup

Console output with full debug information:

```cpp
gQtLogger
    .addSeqNumber()
    .formatPretty(true, 20)  // Colorized, category width 20
    .sendToStdErr();

gQtLogger.installMessageHandler();
```

### Production Setup

File logging with rotation, warnings only to stderr:

```cpp
gQtLogger
    .moveToOwnThread()  // Non-blocking
    
    .pipeline()
        .filterLevel(QtWarningMsg)
        .formatPretty(true)
        .sendToStdErr()
    .end()
    
    .pipeline()
        .format("%{time yyyy-MM-dd hh:mm:ss.zzz} [%{type}] [%{category}] %{file}:%{line} %{message}")
        .sendToFile("app.log", 50*1024*1024, 10,  // 50MB, 10 files
                    QtLogger::RotatingFileSink::RotationDaily
                    | QtLogger::RotatingFileSink::Compression)
    .end();

gQtLogger.installMessageHandler();
```

### Centralized Logging

Send logs to a central server:

```cpp
gQtLogger
    .moveToOwnThread()
    .addSeqNumber()
    .addAppInfo()
    .addHostInfo()
    
    .pipeline()
        .formatPretty()
        .sendToStdErr()
    .end()
    
    .pipeline()
        .filter("^(?!.*password).*$")  // Exclude sensitive data
        .formatToJson(true)            // Compact JSON
        .sendToHttp("https://logs.company.com/api/v1/logs")
    .end();

gQtLogger.installMessageHandler();
```

### Debug-Only Detailed Logging

```cpp
gQtLogger
#ifdef QT_DEBUG
    .format("%{time hh:mm:ss.zzz} %{type} [%{category}] %{shortfile}:%{line} %{func}: %{message}")
#else
    .filterLevel(QtInfoMsg)
    .format("%{time} [%{type}] %{message}")
#endif
    .sendToStdErr();

gQtLogger.installMessageHandler();
```

### INI File for Development

```ini
[logger]
async = false
message_pattern = "%{time hh:mm:ss.zzz} %{type:^8} [%{category:<15}] %{shortfile}:%{line} - %{message}"
stderr = true
stderr_color = true
```

### INI File for Production

```ini
[logger]
async = true
filter_rules = "*.debug=false"
message_pattern = "%{time yyyy-MM-dd hh:mm:ss.zzz} [%{type}] [%{category}] %{message}"
path = "/var/log/myapp/app.log"
max_file_size = 52428800
max_file_count = 10
rotate_daily = true
compress_old_files = true
```

---

## Navigation

| Previous | Next |
|----------|------|
| [← Architecture](architecture.md) | [API Reference →](api/index.md) |
[QtLogger Docs](index.md) > Getting Started

# Getting Started

This guide will help you integrate QtLogger into your Qt project and get logging up and running quickly.

---

## Table of Contents

- [Installation](#installation)
  - [Option 1: Header-Only (Simplest)](#option-1-header-only-simplest)
  - [Option 2: CMake](#option-2-cmake)
  - [Option 3: qmake](#option-3-qmake)
- [Hello World](#hello-world)
- [Preprocessor Macros](#preprocessor-macros)
- [Next Steps](#next-steps)

---

## Installation

QtLogger offers multiple integration options depending on your project's needs.

### Option 1: Header-Only (Simplest)

The simplest approach is to use QtLogger as a single header file. Just copy [`qtlogger.h`](https://github.com/yamixst/qtlogger/raw/refs/heads/main/qtlogger.h) to your project directory and include it:

```cpp
#include "qtlogger.h"
```

This approach requires no build configuration changes and works immediately.

### Option 2: CMake

For CMake-based projects, add QtLogger as a subdirectory and link against it:

```cmake
# Add QtLogger subdirectory
add_subdirectory(path/to/qtlogger)

# Link your target
target_link_libraries(your_target PRIVATE qtlogger)
```

### Option 3: qmake

For qmake projects, include the QtLogger `.pri` file in your `.pro` file:

```qmake
# In your project's .pro file
include(path/to/qtlogger/qtlogger_link.pri)
```

---

## Hello World

Here's the minimal example to get QtLogger working:

```cpp
#include <QCoreApplication>
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Configure with default settings (pretty colored output to stderr)
    gQtLogger.configure();
    
    // Use standard Qt logging macros - they now go through QtLogger
    qDebug() << "Debug message";
    qInfo() << "Info message";
    qWarning() << "Warning message";
    qCritical() << "Critical message";
    
    return app.exec();
}
```

The `gQtLogger.configure()` call sets up:
- Colored pretty output to stderr
- A formatter with timestamp, log level, category, and message

### Logging to a File

To add file logging with rotation:

```cpp
#include <QCoreApplication>
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Configure with rotating file output
    gQtLogger.configure("app.log",           // Log file path
                        1024 * 1024,          // Max file size (1 MB)
                        5,                    // Keep 5 rotated files
                        QtLogger::RotatingFileSink::Compression);  // Compress old files
    
    qDebug() << "Application started";
    
    return app.exec();
}
```

### Using Fluent API

For more control, use the fluent API:

```cpp
#include <QCoreApplication>
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    gQtLogger
        .filterLevel(QtWarningMsg)           // Only warnings and above
        .format("%{time} [%{type}] %{message}")
        .sendToStdErr();
    
    // Important: call this when using fluent API
    gQtLogger.installMessageHandler();
    
    qDebug() << "This won't be shown";       // Filtered out
    qWarning() << "This will be shown";
    
    return app.exec();
}
```

> **Note**: When using the fluent API instead of `configure()`, you must explicitly call `gQtLogger.installMessageHandler()` to activate the logger.

---

## Preprocessor Macros

QtLogger is modular and uses preprocessor macros to enable/disable features. This allows you to include only what you need and reduce binary size.

### Feature Macros

| Macro | Description |
|-------|-------------|
| `QTLOGGER_NO_THREAD` | Disable threading support. Removes mutex locks and async logging capability. Use this for single-threaded applications. |
| `QTLOGGER_NETWORK` | Enable network features: `HttpSink` for sending logs to HTTP endpoints, and `HostInfoAttrs` for adding hostname/IP to log messages. |
| `QTLOGGER_SYSLOG` | Enable Unix syslog support (`SyslogSink`). Linux/Unix only. |
| `QTLOGGER_SDJOURNAL` | Enable systemd journal support (`SdJournalSink`). Linux only. Requires `libsystemd`. |
| `QTLOGGER_ANDROIDLOG` | Enable Android logcat support (`AndroidLogSink`). Automatically defined on Android. |
| `QTLOGGER_OSLOG` | Enable macOS/iOS os_log support (`OslogSink`). Automatically defined on Apple platforms. |

### How to Define Macros

**CMake:**

```cmake
target_compile_definitions(your_target PRIVATE 
    QTLOGGER_NETWORK 
    QTLOGGER_SYSLOG
)
```

**qmake:**

```qmake
DEFINES += QTLOGGER_NETWORK QTLOGGER_SYSLOG
```

**Header-only:**

Define macros before including the header:

```cpp
#define QTLOGGER_NETWORK
#define QTLOGGER_NO_THREAD
#include "qtlogger.h"
```

### Platform Detection

Some macros are automatically defined based on the target platform:

| Platform | Auto-defined Macros |
|----------|-------------------|
| Android | `QTLOGGER_ANDROIDLOG` |
| macOS/iOS | `QTLOGGER_OSLOG` |
| Linux with systemd | (requires manual `QTLOGGER_SDJOURNAL`) |
| Unix-like systems | (requires manual `QTLOGGER_SYSLOG`) |

---

## Using Logging Categories

QtLogger fully supports Qt's logging categories:

```cpp
#include <QCoreApplication>
#include <QLoggingCategory>
#include "qtlogger.h"

// Define logging categories
Q_LOGGING_CATEGORY(lcNetwork, "network")
Q_LOGGING_CATEGORY(lcDatabase, "database")

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    gQtLogger.configure();
    
    // Use category-specific logging
    qCDebug(lcNetwork) << "Connecting to server...";
    qCInfo(lcDatabase) << "Database initialized";
    qCWarning(lcNetwork) << "Connection timeout";
    
    return app.exec();
}
```

Categories can be filtered using `filterCategory()`:

```cpp
gQtLogger
    .filterCategory("network.debug=false")  // Disable network debug logs
    .formatPretty()
    .sendToStdErr();

gQtLogger.installMessageHandler();
```

---

## Next Steps

Now that you have QtLogger running, explore these topics:

| Topic | Description |
|-------|-------------|
| [Architecture](architecture.md) | Understand how pipelines, handlers, and message flow work |
| [Configuration](configuration.md) | Learn all configuration options including INI files |
| [API Reference](api/index.md) | Complete reference for all classes and methods |
| [Advanced Usage](advanced.md) | Async logging, thread safety, custom handlers |

---

## Navigation

| Previous | Next |
|----------|------|
| [← Index](index.md) | [Architecture →](architecture.md) |
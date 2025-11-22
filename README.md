![Project Status](https://img.shields.io/badge/status-beta-orange)

# Qt Logger

A simple yet powerful logging solution for the Qt Framework. This project is designed to provide developers with an intuitive and configurable logging system for Qt-based applications. QtLogger features a customizable pipeline for processing log messages through the `qInstallMessageHandler()` function, allowing for flexible handling and output of logs.

## Key Features

### Drop-in Replacement
- **Zero code changes** - works with existing `qDebug()`, `qInfo()`, `qWarning()`, `qCritical()`, `qFatal()`
- One-line configuration: `gQtLogger.configure()`
- Full support for `Q_LOGGING_CATEGORY`

### Multiple Output Destinations (Sinks)
- **Console**: stdout, stderr with optional **colored output** (ANSI escape codes, auto-detects TTY)
- **Files**: Simple files or rotating logs with size-based, daily, and startup rotation + gzip compression
- **Platform-native**: Android logcat, macOS/iOS os_log, Linux syslog/systemd journal, Windows debugger
- **Network**: HTTP endpoints for remote logging
- **Custom**: Qt signals, any QIODevice

### Powerful Filtering
- **Category-based**: Qt standard filter rules (`*.debug=false`)
- **Regex patterns**: Filter by message content
- **Level-based**: Minimum log level filtering
- **Duplicate suppression**: Prevent log spam
- **Custom filters**: Lambda/function-based filtering

### Flexible Formatting
- **Pattern formatter**: Rich placeholder support with advanced formatting
  - Time formats: ISO 8601, custom patterns, process/boot time
  - Fixed-width fields with alignment and truncation
  - Conditional blocks based on log level
  - Custom attributes support
- **JSON formatter**: Structured logging for log aggregation
- **Pretty formatter**: Human-readable with colors and alignment
- **Custom formatters**: Full control with lambda functions

### Pipeline Architecture
- **Chainable API**: Fluent configuration style
- **Multiple pipelines**: Different configurations for different outputs
- **Custom attributes**: Enrich messages with metadata (sequence numbers, app info, user data)
- **Sequential processing**: Attributes → Filters → Formatters → Sinks

### Performance
- **Thread-safe**: Safe concurrent logging from multiple threads
- **Asynchronous logging**: Optional dedicated thread for non-blocking I/O
- **Efficient**: Minimal overhead on application performance

### Configuration Options
- **Programmatic**: Fluent API for code-based setup
- **File-based**: INI file configuration for runtime flexibility
- **Environment variables**: Standard Qt logging environment support

### Cross-Platform
- Linux, Windows, macOS, iOS, Android
- Qt 5.9 - Qt 6.x
- C++17 compatible

## Integration

There are multiple ways to integrate QtLogger into your project:

### Option 1: Header-Only (Simplest)

Just copy [qtlogger.h](https://github.com/yamixst/qtlogger/raw/refs/heads/main/qtlogger.h) to your project and include it:

```cpp
#include "qtlogger.h"
```

This is the simplest approach - no build configuration needed.

### Option 2: CMake

Link against the QtLogger library in your `CMakeLists.txt`:

```cmake
# Add QtLogger subdirectory
add_subdirectory(path/to/qtlogger)

# Link your target
target_link_libraries(your_target PRIVATE qtlogger)
```

### Option 3: qmake

Include the QtLogger `.pri` file in your `.pro` file:

```qmake
# In your project's .pro file
include(path/to/qtlogger/qtlogger_link.pri)
```

## Quick start

Use the global gQtLogger object for configuration:

```cpp
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    gQtLogger.configure();  // That's it!
    
    qDebug() << "It just works!";
    
    return app.exec();
}
```

### Advanced Example

```cpp
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Configure with multiple pipelines
    gQtLogger
        .moveToOwnThread()                 // Asynchronous logging
        .addSeqNumber()
        .pipeline()
            .filterLevel(QtWarningMsg)     // Only warnings and above
            .filterDuplicate()
            .formatPretty()
            .sendToStdErr(true)            // Colored output
        .end()
        .pipeline()
            .format("%{time} [%{category}] %{type}: %{message}")
            .sendToFile("app.log", 1024 * 1024, 5,  // Rotating logs (1MB, 5 files)
                        QtLogger::RotatingFileSink::RotationOnStartup
                        | QtLogger::RotatingFileSink::RotationDaily
                        | QtLogger::RotatingFileSink::Compression)
        .end()
        .pipeline()
            .addAppInfo()
            .addHostInfo()
            .filter("^(?!.*password:).*$")  // Filter out sensitive data via Regex
            .formatToJson()
            .sendToHttp(QUrl("https://logs.example.com"))
        .end();

    gQtLogger.installMessageHandler();

    qDebug() << "Application started";
    qWarning() << "This is a warning";

    return app.exec();
}
```

### Configuration from File

```cpp
// Load configuration from INI file
gQtLogger.configure("config.ini");
```

Example `config.ini`:
```ini
[logger]
filter_rules = "*.debug=false"
message_pattern = "%{time} [%{category}] %{type}: %{message}"
stdout = true
stdout_color = true
path = "app.log"
max_file_size = 1048576
max_file_count = 5
rotate_on_startup = true
rotate_daily = false
compress_old_files = false
async = true
```

## Requirements

- **C++17 compatible compiler**: QtLogger requires a compiler with full C++17 standard support
- **Qt Framework**: Qt 5.9 - Qt 6.x

### Supported Compilers

- GCC 7.0 or later
- Clang 5.0 or later
- MSVC 2017 (Visual Studio 15.0) or later
- Apple Clang (Xcode 10.0) or later

## Documentation

- [**FEATURES.md**](doc/FEATURES.md) - Comprehensive list of all features and capabilities
- [**ARCHITECTURE.md**](doc/ARCHITECTURE.md) - Internal architecture and design patterns
- Examples in the `examples/` directory

## No Code Changes Required

You don't need to modify your existing logging code if you're using standard Qt logging functions like `qDebug()`, `qInfo()`, `qWarning()`, `qCritical()`, or `qFatal()`. Simply add one line of code to your `main()` function to configure QtLogger, and all your existing log statements will automatically work with the new logging system.

## Versioning

This project adheres to **Semantic Versioning 2.0.0** (SemVer). 

Semantic Versioning is a versioning scheme that uses a three-part version number: `MAJOR.MINOR.PATCH`, where:

- **MAJOR** version changes introduce incompatible API changes.
- **MINOR** version changes add functionality in a backward-compatible manner.
- **PATCH** version changes include backward-compatible bug fixes.

For more details, refer to the [Semantic Versioning specification](https://semver.org/). 

By following SemVer, we aim to provide clear and predictable version updates for our users.

## License

This project is licensed under the MIT License. See the [LICENSE](./LICENSE) file for details.

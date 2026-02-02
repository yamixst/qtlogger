[QtLogger Documentation](index.md)

# QtLogger Documentation

![Project Status](https://img.shields.io/badge/status-beta-orange)
![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt Version](https://img.shields.io/badge/Qt-5.9--6.10-green)
![License](https://img.shields.io/badge/license-MIT-brightgreen)

A simple yet powerful logging solution for the Qt Framework. QtLogger provides an intuitive and configurable logging system that works seamlessly with existing Qt logging macros (`qDebug()`, `qInfo()`, `qWarning()`, `qCritical()`, `qFatal()`).

## Key Features

- **Zero code changes** — works with existing Qt logging macros
- **One-line configuration**: `gQtLogger.configure()`
- **Flexible pipeline architecture** for processing log messages
- **Multiple output destinations** (console, file, network, system logs)
- **Rich formatting options** with pattern and JSON formatters
- **Category-based and regex filtering**
- **Asynchronous logging** with optional dedicated thread
- **Cross-platform**: Linux, Windows, macOS, iOS, Android

---

## Table of Contents

### Getting Started

- **[Getting Started](getting_started.md)** — Installation, integration options, Hello World example, and preprocessor macros

### Core Concepts

- **[Architecture](architecture.md)** — Understanding the pipeline concept, data flow, handler types, and lifecycle management

### Configuration

- **[Configuration Guide](configuration.md)** — Fluent API, INI file configuration, and Qt logging rules integration

### API Reference

- **[API Reference](api/index.md)** — Complete API documentation
  - [Core Classes](api/core.md) — `Logger`, `LogMessage`, context and utilities
  - [Pipelines](api/pipelines.md) — `Pipeline`, `SimplePipeline`, `SortedPipeline`
  - [Sinks](api/sinks.md) — All output destinations (File, Console, Network, Syslog, etc.)
  - [Formatters](api/formatters.md) — Pattern, JSON, Pretty, and Qt default formatters
  - [Filters](api/filters.md) — Level, Category, RegExp, and Duplicate filters
  - [Attribute Handlers](api/attributes.md) — Custom attributes, AppInfo, HostInfo, SeqNumber

### Advanced Topics

- **[Advanced Usage](advanced.md)** — Asynchronous logging, thread safety, and creating custom handlers

---

## Quick Example

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

For more complex scenarios:

```cpp
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    gQtLogger
        .moveToOwnThread()                 // Asynchronous logging
        .addSeqNumber()
        .pipeline()
            .filterLevel(QtWarningMsg)     // Only warnings and above
            .formatPretty()
            .sendToStdErr(true)            // Colored output
        .end()
        .pipeline()
            .format("%{time} [%{category}] %{type}: %{message}")
            .sendToFile("app.log", 1024 * 1024, 5)
        .end();

    gQtLogger.installMessageHandler();

    qDebug() << "Application started";
    qWarning() << "This is a warning";

    return app.exec();
}
```

---

## Requirements

- **C++17** compatible compiler
- **Qt Framework**: Qt 5.9 — Qt 6.10

---

## License

This project is licensed under the MIT License. See the [LICENSE](../LICENSE) file for details.

---

## Navigation

| Next |
|------|
| [Getting Started →](getting_started.md) |
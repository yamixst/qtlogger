[QtLogger Docs](../index.md) > [API Reference](index.md) > Formatters

# Formatters

Formatters convert `LogMessage` objects into formatted strings for output. This section documents all available formatter classes and the pattern placeholder syntax.

---

## Table of Contents

- [Formatter (Base Class)](#formatter-base-class)
- [PatternFormatter](#patternformatter)
  - [Basic Placeholders](#basic-placeholders)
  - [Time Placeholders](#time-placeholders)
  - [Custom Attributes](#custom-attributes)
  - [Fixed-Width Formatting](#fixed-width-formatting)
  - [Conditional Blocks](#conditional-blocks)
- [JsonFormatter](#jsonformatter)
- [PrettyFormatter](#prettyformatter)
- [QtLogMessageFormatter](#qtlogmessageformatter)
- [FunctionFormatter](#functionformatter)

---

## Formatter (Base Class)

The abstract base class for all formatters.

### Inheritance

```
Handler
└── Formatter
```

### Description

`Formatter` provides the interface for converting log messages to strings. All formatter implementations must override the `format()` method.

### Virtual Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `format(const LogMessage &lmsg)` | `QString` | **Pure virtual.** Convert message to string |

### Inherited Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `type()` | `HandlerType` | Returns `HandlerType::Formatter` |
| `process(LogMessage &lmsg)` | `bool` | Calls `format()` and sets `lmsg.formattedMessage()` |

### Example: Custom Formatter

```cpp
#include "qtlogger.h"

class XmlFormatter : public QtLogger::Formatter
{
public:
    QString format(const QtLogger::LogMessage &lmsg) override
    {
        return QString("<log level=\"%1\" time=\"%2\"><![CDATA[%3]]></log>\n")
            .arg(QtLogger::qtMsgTypeToString(lmsg.type()))
            .arg(lmsg.time().toString(Qt::ISODate))
            .arg(lmsg.message());
    }
};
```

---

## PatternFormatter

A highly configurable formatter using placeholder patterns.

### Inheritance

```
Handler
└── Formatter
    └── PatternFormatter
```

### Constructor

```cpp
explicit PatternFormatter(const QString &pattern);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `pattern` | `QString` | Format pattern with placeholders |

### SimplePipeline Method

```cpp
SimplePipeline &format(const QString &pattern);
```

### Example

```cpp
#include "qtlogger.h"

gQtLogger
    .format("%{time yyyy-MM-dd hh:mm:ss.zzz} [%{type:>8}] [%{category}] %{message}")
    .sendToStdErr();

gQtLogger.installMessageHandler();
```

---

### Basic Placeholders

| Placeholder | Description | Example Output |
|-------------|-------------|----------------|
| `%{message}` | Log message text | `Application started` |
| `%{type}` | Message type name | `debug`, `info`, `warning`, `critical`, `fatal` |
| `%{category}` | Logging category name | `default`, `network`, `app.ui` |
| `%{file}` | Full source file path | `/home/user/project/src/main.cpp` |
| `%{shortfile}` | File name only (no path) | `main.cpp` |
| `%{shortfile /base/path}` | File relative to base path | `src/main.cpp` |
| `%{line}` | Source line number | `42` |
| `%{function}` | Full function signature | `void MyClass::myMethod(int, QString)` |
| `%{func}` | Cleaned function name | `MyClass::myMethod` |
| `%{threadid}` | Thread ID | `140234567890` |
| `%{qthreadptr}` | QThread pointer (hex) | `0x7f8a1c002340` |

### Time Placeholders

| Placeholder | Description | Example Output |
|-------------|-------------|----------------|
| `%{time}` | ISO 8601 format | `2024-01-15T14:30:45.123` |
| `%{time FORMAT}` | Custom Qt date/time format | See below |
| `%{time process}` | Seconds since process start | `12.345` |
| `%{time boot}` | Seconds since system boot | `86412.789` |

#### Time Format Specifiers

Use Qt's date/time format specifiers:

| Specifier | Description | Example |
|-----------|-------------|---------|
| `yyyy` | 4-digit year | `2024` |
| `MM` | 2-digit month | `01` |
| `dd` | 2-digit day | `15` |
| `hh` | 2-digit hour (24h) | `14` |
| `mm` | 2-digit minute | `30` |
| `ss` | 2-digit second | `45` |
| `zzz` | 3-digit millisecond | `123` |

**Examples:**

```
%{time yyyy-MM-dd hh:mm:ss}     → 2024-01-15 14:30:45
%{time hh:mm:ss.zzz}            → 14:30:45.123
%{time dd.MM.yyyy}              → 15.01.2024
%{time yyyy-MM-ddThh:mm:ss.zzz} → 2024-01-15T14:30:45.123
```

---

### Custom Attributes

Access custom attributes added by attribute handlers:

| Placeholder | Description |
|-------------|-------------|
| `%{attr_name}` | Attribute value (error if missing) |
| `%{attr_name?}` | Optional attribute (empty if missing) |
| `%{attr_name?N}` | Optional, remove N chars before if missing |
| `%{attr_name?N,M}` | Optional, remove N chars before and M chars after if missing |

#### Optional Attribute Syntax

The `?N,M` syntax is useful for removing surrounding punctuation when an attribute is not present:

```
Pattern: "#%{seq_number?1} %{message}"

With seq_number=42:    "#42 Hello"
Without seq_number:    " Hello"     (# removed because of ?1)

Pattern: "[%{user?1,1}] %{message}"

With user="admin":     "[admin] Hello"
Without user:          " Hello"         ([ and ] removed)
```

**Examples:**

```cpp
gQtLogger
    .addSeqNumber()
    .addAppInfo()
    .format("#%{seq_number:0>6} [%{appname}] %{message}")
    .sendToStdErr();

// Output: #000001 [MyApp] Application started
```

---

### Fixed-Width Formatting

Any placeholder can have a format specification after a colon for alignment and padding.

#### Syntax

```
%{placeholder:[fill][align][width][!]}
```

| Component | Description |
|-----------|-------------|
| `fill` | Single character for padding (default: space) |
| `align` | Alignment direction: `<` left, `>` right, `^` center |
| `width` | Minimum field width |
| `!` | Enable truncation (max width) |

#### Alignment Characters

| Char | Alignment | Truncation Behavior |
|------|-----------|---------------------|
| `<` | Left | Keep first N characters |
| `>` | Right | Keep last N characters |
| `^` | Center | Keep first N characters |

#### Padding Only (No `!`)

When `!` is not present, values shorter than width are padded, but longer values are NOT truncated.

| Pattern | Input | Output |
|---------|-------|--------|
| `%{type:<10}` | `debug` | `debug     ` |
| `%{type:>10}` | `debug` | `     debug` |
| `%{type:^10}` | `debug` | `  debug   ` |
| `%{type:*<10}` | `debug` | `debug*****` |
| `%{type:0>8}` | `42` | `00000042` |
| `%{type:<5}` | `warning` | `warning` (not truncated) |

#### Truncation Only (`!` Without Fill)

When `!` is present without a fill character, long values are truncated but short values are NOT padded.

| Pattern | Input | Output |
|---------|-------|--------|
| `%{type:10!}` | `critical` | `critical` |
| `%{type:5!}` | `critical` | `criti` |
| `%{type:<5!}` | `critical` | `criti` (first 5 chars) |
| `%{type:>5!}` | `critical` | `tical` (last 5 chars) |

#### Truncation AND Padding (`!` With Fill)

When both fill and `!` are present, values are truncated AND padded to exactly the specified width.

| Pattern | Input | Output |
|---------|-------|--------|
| `%{type: <8!}` | `debug` | `debug   ` |
| `%{type: <8!}` | `critical` | `critical` (truncated to 8) |
| `%{type:*>10!}` | `info` | `******info` |
| `%{type:*>10!}` | `verylongtype` | `verylongty` |

#### Examples

```cpp
// Fixed-width log level column
gQtLogger
    .format("%{time hh:mm:ss} %{type:>8} | %{message}")
    .sendToStdErr();

// Output:
// 14:30:45    debug | Debug message
// 14:30:46     info | Info message  
// 14:30:47  warning | Warning message
// 14:30:48 critical | Critical message

// Zero-padded sequence numbers
gQtLogger
    .addSeqNumber()
    .format("#%{seq_number:0>6} %{message}")
    .sendToStdErr();

// Output:
// #000001 First message
// #000002 Second message
// #000123 Message 123

// Truncated category with fixed width
gQtLogger
    .format("[%{category:<10!}] %{message}")
    .sendToStdErr();

// Output:
// [default   ] Message
// [network   ] Message
// [app.ui.dia] Message (truncated from app.ui.dialogs)
```

---

### Conditional Blocks

Include content only for specific message types.

| Block | Shows for |
|-------|-----------|
| `%{if-debug}...%{endif}` | Debug messages only |
| `%{if-info}...%{endif}` | Info messages only |
| `%{if-warning}...%{endif}` | Warning messages only |
| `%{if-critical}...%{endif}` | Critical messages only |
| `%{if-fatal}...%{endif}` | Fatal messages only |

#### Example

```cpp
gQtLogger
    .format("%{time hh:mm:ss} "
            "%{if-debug}D%{endif}"
            "%{if-info}I%{endif}"
            "%{if-warning}W%{endif}"
            "%{if-critical}E%{endif}"
            "%{if-fatal}F%{endif}"
            " [%{category}] %{message}")
    .sendToStdErr();

// Output:
// 14:30:45 D [default] Debug message
// 14:30:46 I [default] Info message
// 14:30:47 W [network] Warning message
// 14:30:48 E [network] Error message
```

#### Conditional Content

The content between `%{if-*}` and `%{endif}` can include any text and other placeholders:

```cpp
gQtLogger
    .format("%{time} %{type}: %{message}"
            "%{if-warning} (check this)%{endif}"
            "%{if-critical} [FILE: %{shortfile}:%{line}]%{endif}")
    .sendToStdErr();
```

---

### Special Characters

| Sequence | Output |
|----------|--------|
| `%%` | Literal `%` |

---

## JsonFormatter

Outputs log messages as JSON objects.

### Inheritance

```
Handler
└── Formatter
    └── JsonFormatter
```

### Constructor

```cpp
explicit JsonFormatter(bool compact = false);
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `compact` | `bool` | `false` | If `true`, output single-line JSON |

### Static Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `instance()` | `JsonFormatterPtr` | Get singleton instance (non-compact) |

### SimplePipeline Method

```cpp
SimplePipeline &formatToJson(bool compact = false);
```

### Output Format

The JSON output includes all attributes from `LogMessage::allAttributes()`:

```json
{
    "type": "warning",
    "time": "2024-01-15T14:30:45.123",
    "category": "network",
    "file": "/path/to/file.cpp",
    "line": 42,
    "function": "void MyClass::myMethod()",
    "message": "Connection timeout",
    "threadId": 140234567890,
    "seq_number": 1,
    "appname": "MyApp"
}
```

Compact format outputs the same data on a single line.

### Example

```cpp
#include "qtlogger.h"

// Compact JSON for log aggregation
gQtLogger
    .addSeqNumber()
    .addAppInfo()
    .addHostInfo()
    .formatToJson(true)
    .sendToHttp("https://logs.example.com/ingest");

gQtLogger.installMessageHandler();

// Pretty JSON to file
gQtLogger
    .formatToJson(false)
    .sendToFile("structured.log");
```

---

## PrettyFormatter

A human-readable formatter with optional ANSI colors.

### Inheritance

```
Handler
└── Formatter
    └── PrettyFormatter
```

### Constructor

```cpp
explicit PrettyFormatter(bool colorize = false, int maxCategoryWidth = 15);
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `colorize` | `bool` | `false` | Enable ANSI color codes |
| `maxCategoryWidth` | `int` | `15` | Maximum width for category column |

### Static Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `instance()` | `PrettyFormatterPtr` | Get singleton instance (non-colorized) |

### SimplePipeline Method

```cpp
SimplePipeline &formatPretty(bool colorize = false, int maxCategoryWidth = 15);
```

### Output Format

```
DD.MM.YYYY hh:mm:ss.zzz T [category    ] message
```

Where `T` is a single-letter type indicator:
- ` ` (space) for debug
- `I` for info
- `W` for warning
- `E` for critical/error
- `F` for fatal

### Color Scheme (When Enabled)

| Component | Color |
|-----------|-------|
| Timestamp | Dark gray |
| Debug messages | Dark gray |
| Info indicator | Green |
| Warning indicator | Orange |
| Error indicator | Bold red |
| Fatal indicator | Dark bold red |
| Category | Normal |
| Message | Normal |

### Thread Tracking

`PrettyFormatter` assigns short numeric indices to threads for easier identification in multi-threaded output:

```
15.01.2024 14:30:45.123 I 1 [network    ] Connected
15.01.2024 14:30:45.124 I 2 [database   ] Query started
15.01.2024 14:30:45.125 W 1 [network    ] Slow response
```

### Example

```cpp
#include "qtlogger.h"

// Development console output
gQtLogger
    .formatPretty(true, 20)  // Colorized, 20-char category width
    .sendToStdErr();

gQtLogger.installMessageHandler();
```

---

## QtLogMessageFormatter

Uses Qt's built-in message formatting via `qFormatLogMessage()`.

### Inheritance

```
Handler
└── Formatter
    └── QtLogMessageFormatter
```

### Constructor

Private. Use `instance()` to get the singleton.

### Static Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `instance()` | `QtLogMessageFormatterPtr` | Get singleton instance |

### SimplePipeline Method

```cpp
SimplePipeline &formatByQt();
```

### Description

This formatter uses Qt's `qFormatLogMessage()` function, which respects:
- `QT_MESSAGE_PATTERN` environment variable
- `qSetMessagePattern()` calls
- `QtLogger::setMessagePattern()` calls

### Example

```cpp
#include "qtlogger.h"

// Use Qt's default formatting
QtLogger::setMessagePattern("[%{type}] %{message}");

gQtLogger
    .formatByQt()
    .sendToStdErr();

gQtLogger.installMessageHandler();
```

---

## FunctionFormatter

A formatter that uses a custom function for formatting.

### Inheritance

```
Handler
└── Formatter
    └── FunctionFormatter
```

### Type Alias

```cpp
using Function = std::function<QString(const LogMessage &)>;
```

### Constructor

```cpp
FunctionFormatter(const Function &func);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `func` | `Function` | Formatting function |

### SimplePipeline Method

```cpp
SimplePipeline &format(std::function<QString(const LogMessage &)> func);
```

### Example

```cpp
#include "qtlogger.h"

// Custom formatting with lambda
gQtLogger
    .format([](const QtLogger::LogMessage &lmsg) {
        return QString("[%1] %2: %3\n")
            .arg(lmsg.time().toString("hh:mm:ss"))
            .arg(lmsg.category())
            .arg(lmsg.message());
    })
    .sendToStdErr();

gQtLogger.installMessageHandler();

// Post-processing another formatter's output
gQtLogger
    .formatPretty(true)
    .format([](const QtLogger::LogMessage &lmsg) {
        // Mask sensitive data in already-formatted message
        QString msg = lmsg.formattedMessage();
        static QRegularExpression re("password=\\S+");
        msg.replace(re, "password=***");
        return msg;
    })
    .sendToStdErr();
```

---

## Complete Pattern Reference

### Quick Reference Table

| Category | Placeholders |
|----------|--------------|
| **Message** | `%{message}` |
| **Type** | `%{type}` |
| **Location** | `%{file}`, `%{shortfile}`, `%{line}`, `%{function}`, `%{func}` |
| **Category** | `%{category}` |
| **Time** | `%{time}`, `%{time FORMAT}`, `%{time process}`, `%{time boot}` |
| **Thread** | `%{threadid}`, `%{qthreadptr}` |
| **Attributes** | `%{name}`, `%{name?}`, `%{name?N}`, `%{name?N,M}` |
| **Conditional** | `%{if-debug}`, `%{if-info}`, `%{if-warning}`, `%{if-critical}`, `%{if-fatal}`, `%{endif}` |
| **Formatting** | `:[fill][align][width][!]` |
| **Escape** | `%%` |

### Common Patterns

**Simple timestamp and message:**
```
%{time hh:mm:ss} %{message}
```

**Full debug information:**
```
%{time yyyy-MM-dd hh:mm:ss.zzz} [%{type:>8}] [%{category}] %{shortfile}:%{line} %{func}: %{message}
```

**Compact for production:**
```
%{time} [%{type}] %{message}
```

**With sequence number:**
```
#%{seq_number:0>6} %{time hh:mm:ss.zzz} %{type}: %{message}
```

**JSON-like (but not actual JSON):**
```
{"time":"%{time}","level":"%{type}","msg":"%{message}"}
```

---

## Navigation

| Previous | Next |
|----------|------|
| [← Sinks](sinks.md) | [Filters →](filters.md) |
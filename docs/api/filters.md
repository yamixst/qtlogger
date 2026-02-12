[QtLogger Docs](../index.md) > [API Reference](index.md) > Filters

# Filters

Filters are handlers that decide whether a log message should continue through the pipeline or be discarded. This section documents all available filter classes.

---

## Table of Contents

- [Filter (Base Class)](#filter-base-class)
- [LevelFilter](#levelfilter)
- [CategoryFilter](#categoryfilter)
- [RegExpFilter](#regexpfilter)
- [DuplicateFilter](#duplicatefilter)
- [FunctionFilter](#functionfilter)

---

## Filter (Base Class)

The abstract base class for all filters.

### Inheritance

```
Handler
└── Filter
```

### Description

`Filter` provides the interface for deciding whether a log message should continue processing. All filter implementations must override the `filter()` method.

### Virtual Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `filter(const LogMessage &lmsg)` | `bool` | **Pure virtual.** Return `true` to pass, `false` to drop |

### Inherited Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `type()` | `HandlerType` | Returns `HandlerType::Filter` |
| `process(LogMessage &lmsg)` | `bool` | Calls `filter()` and returns the result |

### Example: Custom Filter

```cpp
#include "qtlogger.h"

class KeywordFilter : public QtLogger::Filter
{
public:
    KeywordFilter(const QString &keyword) : m_keyword(keyword) {}
    
    bool filter(const QtLogger::LogMessage &lmsg) override
    {
        // Only pass messages containing the keyword
        return lmsg.message().contains(m_keyword, Qt::CaseInsensitive);
    }

private:
    QString m_keyword;
};

// Usage
auto filter = QSharedPointer<KeywordFilter>::create("error");
gQtLogger << filter;
```

---

## LevelFilter

Filters messages by minimum severity level.

### Inheritance

```
Handler
└── Filter
    └── LevelFilter
```

### Description

`LevelFilter` passes only messages with a severity level equal to or higher than the specified minimum level. This is one of the most commonly used filters.

### Constructor

```cpp
explicit LevelFilter(QtMsgType minLevel = QtDebugMsg);
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `minLevel` | `QtMsgType` | `QtDebugMsg` | Minimum level to pass |

### Level Priority

| Level | Priority | Passes when minLevel is... |
|-------|----------|---------------------------|
| `QtDebugMsg` | 0 | Debug |
| `QtInfoMsg` | 1 | Debug, Info |
| `QtWarningMsg` | 2 | Debug, Info, Warning |
| `QtCriticalMsg` | 3 | Debug, Info, Warning, Critical |
| `QtFatalMsg` | 4 | Any |

### SimplePipeline Method

```cpp
SimplePipeline &filterLevel(QtMsgType minLevel);
```

### Example

```cpp
#include "qtlogger.h"

// Only warnings and above
gQtLogger
    .filterLevel(QtWarningMsg)
    .formatPretty()
    .sendToStdErr();

gQtLogger.installMessageHandler();

qDebug() << "This is hidden";      // Filtered out
qInfo() << "This is hidden";       // Filtered out
qWarning() << "This is shown";     // Passes
qCritical() << "This is shown";    // Passes

// Manual creation
auto filter = QtLogger::LevelFilterPtr::create(QtInfoMsg);
gQtLogger << filter;
```

### Use Cases

- **Production logging**: Filter out debug messages
- **Error-only logs**: Capture only critical issues to a separate file
- **Development**: Show all messages including debug

```cpp
// Production: Only info and above
gQtLogger
    .filterLevel(QtInfoMsg)
    .format("%{time} [%{type}] %{message}")
    .sendToFile("app.log");

// Error log: Only critical and fatal
gQtLogger
    .pipeline()
        .filterLevel(QtCriticalMsg)
        .format("%{time} %{file}:%{line} %{message}")
        .sendToFile("errors.log")
    .end();
```

---

## CategoryFilter

Filters messages based on Qt logging category rules.

### Inheritance

```
Handler
└── Filter
    └── CategoryFilter
```

### Description

`CategoryFilter` uses Qt-style filter rules to enable or disable messages based on their logging category and type. This provides fine-grained control over which categories produce output.

### Constructor

```cpp
CategoryFilter(const QString &rules);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `rules` | `QString` | Filter rules string |

### Rule Syntax

```
[category][.type]=true|false
```

Multiple rules are separated by semicolons (`;`) or newlines.

| Component | Description |
|-----------|-------------|
| `category` | Category name or `*` for all categories |
| `.type` | Optional: `.debug`, `.info`, `.warning`, `.critical` |
| `=true` | Enable matching messages |
| `=false` | Disable matching messages |

### Rule Matching

Rules are processed in order. Later rules override earlier ones for the same category.

| Rule | Effect |
|------|--------|
| `*=true` | Enable all categories |
| `*=false` | Disable all categories |
| `*.debug=false` | Disable debug for all categories |
| `network=true` | Enable all levels for `network` category |
| `network.*=true` | Same as above |
| `network.debug=false` | Disable debug for `network` category |
| `app.ui.*=false` | Disable all for `app.ui` and subcategories |

### Wildcard Matching

- `*` matches any category
- Category names support prefix matching with wildcards
- `network.*` matches `network`, `network.http`, `network.tcp`, etc.

### SimplePipeline Method

```cpp
SimplePipeline &filterCategory(const QString &rules);
```

### Example

```cpp
#include "qtlogger.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcNetwork, "network")
Q_LOGGING_CATEGORY(lcDatabase, "database")
Q_LOGGING_CATEGORY(lcUI, "app.ui")

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Disable all debug, but enable network category
    gQtLogger
        .filterCategory("*.debug=false;network.*=true")
        .formatPretty()
        .sendToStdErr();
    
    gQtLogger.installMessageHandler();
    
    qCDebug(lcNetwork) << "Network debug - shown";
    qCDebug(lcDatabase) << "Database debug - hidden";
    qCInfo(lcDatabase) << "Database info - shown";
    qCDebug(lcUI) << "UI debug - hidden";
    
    return app.exec();
}
```

### Complex Rule Examples

```cpp
// Disable all debug except for specific categories
gQtLogger.filterCategory(
    "*.debug=false;"
    "network.debug=true;"
    "database.debug=true"
);

// Enable only warnings and above, but show all for specific category
gQtLogger.filterCategory(
    "*=false;"
    "*.warning=true;"
    "*.critical=true;"
    "myapp.important=true"
);

// Disable a noisy category entirely
gQtLogger.filterCategory(
    "*=true;"
    "thirdparty.verbose=false"
);
```

---

## RegExpFilter

Filters messages using a regular expression pattern.

### Inheritance

```
Handler
└── Filter
    └── RegExpFilter
```

### Description

`RegExpFilter` passes only messages whose text matches the specified regular expression. This allows for powerful content-based filtering.

### Constructors

```cpp
explicit RegExpFilter(const QRegularExpression &regExp);
explicit RegExpFilter(const QString &regExp);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `regExp` | `QRegularExpression` or `QString` | Regular expression pattern |

### Behavior

- The filter checks if the pattern **matches** the message text
- A match means the message **passes** (is included)
- No match means the message is **dropped** (filtered out)

> **Note**: The regex is matched against the **original message**, not the formatted message.

### SimplePipeline Method

```cpp
SimplePipeline &filter(const QString &regexp);
```

### Example

```cpp
#include "qtlogger.h"

// Only show messages containing "error" or "Error"
gQtLogger
    .filter("[Ee]rror")
    .formatPretty()
    .sendToStdErr();

gQtLogger.installMessageHandler();

qDebug() << "Normal message";     // Filtered out
qDebug() << "Error occurred";     // Passes
qWarning() << "error: timeout";   // Passes
```

### Common Patterns

| Pattern | Effect |
|---------|--------|
| `.*` | Match everything (pass all) |
| `^$` | Match empty messages only |
| `error\|warning` | Match messages containing "error" OR "warning" |
| `^\[IMPORTANT\]` | Match messages starting with "[IMPORTANT]" |
| `(?!.*password).*` | Match messages NOT containing "password" |
| `\d{3}-\d{4}` | Match messages with phone number pattern |

### Excluding Sensitive Data

Use negative lookahead to exclude messages containing sensitive information:

```cpp
// Exclude messages with passwords
gQtLogger
    .filter("^(?!.*password).*$")
    .formatToJson()
    .sendToHttp("https://logs.example.com");

// Exclude messages with credit card patterns
gQtLogger
    .filter("^(?!.*\\d{4}[- ]?\\d{4}[- ]?\\d{4}[- ]?\\d{4}).*$")
    .formatPretty()
    .sendToFile("app.log");
```

### Case-Insensitive Matching

```cpp
// Using QRegularExpression for case-insensitive matching
auto filter = QtLogger::RegExpFilterPtr::create(
    QRegularExpression("error", QRegularExpression::CaseInsensitiveOption)
);
gQtLogger << filter;
```

---

## DuplicateFilter

Suppresses consecutive duplicate messages.

### Inheritance

```
Handler
└── Filter
    └── DuplicateFilter
```

### Description

`DuplicateFilter` filters out messages that are identical to the immediately preceding message. This helps prevent log spam from repeated events.

### Constructor

```cpp
DuplicateFilter();
```

### Behavior

- Compares current message text to the previous message text
- If identical, the message is dropped
- Different messages always pass
- Only consecutive duplicates are filtered (A, A, B, A → A, B, A)

### SimplePipeline Method

```cpp
SimplePipeline &filterDuplicate();
```

### Example

```cpp
#include "qtlogger.h"

gQtLogger
    .filterDuplicate()
    .formatPretty()
    .sendToStdErr();

gQtLogger.installMessageHandler();

// In a loop that might produce duplicate messages
for (int i = 0; i < 100; i++) {
    if (someCondition) {
        qWarning() << "Condition triggered";  // Only logged once if consecutive
    }
}
```

### Use Cases

- **Polling loops**: Prevent repeated status messages
- **Event handlers**: Filter rapid duplicate events
- **Retry logic**: Avoid logging the same error multiple times

```cpp
// Connection retry example
gQtLogger
    .filterDuplicate()  // Suppress repeated "Connection failed" messages
    .formatPretty()
    .sendToStdErr();

gQtLogger.installMessageHandler();

for (int attempt = 0; attempt < 10; attempt++) {
    if (!connect()) {
        qWarning() << "Connection failed";  // Only logged once until success
    }
}
```

### Limitations

- Only filters **consecutive** duplicates
- Non-consecutive duplicates still appear: A, B, A → all three logged
- Compares message text only, not type or category

---

## FunctionFilter

A filter that uses a custom function for filtering logic.

### Inheritance

```
Handler
└── Filter
    └── FunctionFilter
```

### Type Alias

```cpp
using Function = std::function<bool(const LogMessage &)>;
```

### Constructor

```cpp
FunctionFilter(const Function &function);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `function` | `Function` | Filter function returning `true` to pass |

### SimplePipeline Method

```cpp
SimplePipeline &filter(std::function<bool(const LogMessage &)> func);
```

### Example

```cpp
#include "qtlogger.h"

// Custom filter with lambda
gQtLogger
    .filter([](const QtLogger::LogMessage &lmsg) {
        // Only pass messages from specific files
        QString file = QString::fromUtf8(lmsg.file());
        return file.contains("important") || file.contains("critical");
    })
    .formatPretty()
    .sendToStdErr();

gQtLogger.installMessageHandler();
```

### Advanced Examples

**Time-based filtering:**

```cpp
// Only log during business hours
gQtLogger
    .filter([](const QtLogger::LogMessage &lmsg) {
        int hour = lmsg.time().time().hour();
        return hour >= 9 && hour < 17;
    })
    .formatPretty()
    .sendToFile("business_hours.log");
```

**Rate limiting:**

```cpp
// Simple rate limiter
class RateLimiter
{
public:
    bool check() {
        auto now = std::chrono::steady_clock::now();
        if (now - m_lastLog > std::chrono::seconds(1)) {
            m_lastLog = now;
            return true;
        }
        return false;
    }
private:
    std::chrono::steady_clock::time_point m_lastLog;
};

RateLimiter limiter;
gQtLogger
    .filter([&limiter](const QtLogger::LogMessage &) {
        return limiter.check();
    })
    .formatPretty()
    .sendToStdErr();
```

**Attribute-based filtering:**

```cpp
// Filter based on custom attributes
gQtLogger
    .attrHandler([](const QtLogger::LogMessage &lmsg) {
        return QVariantHash{{ "user", getCurrentUser() }};
    })
    .filter([](const QtLogger::LogMessage &lmsg) {
        // Only log messages from admin users
        return lmsg.attribute("user").toString() == "admin";
    })
    .formatPretty()
    .sendToStdErr();
```

**Multi-condition filtering:**

```cpp
gQtLogger
    .filter([](const QtLogger::LogMessage &lmsg) {
        // Complex filtering logic
        if (lmsg.type() >= QtCriticalMsg) {
            return true;  // Always pass critical and fatal
        }
        
        QString category = QString::fromUtf8(lmsg.category());
        if (category.startsWith("internal.")) {
            return false;  // Hide internal categories
        }
        
        if (lmsg.message().length() > 1000) {
            return false;  // Filter out overly long messages
        }
        
        return true;
    })
    .formatPretty()
    .sendToStdErr();
```

---

## Combining Filters

Multiple filters can be combined in a pipeline. Messages must pass **all** filters to be output.

### Sequential Filters

```cpp
gQtLogger
    .filterLevel(QtInfoMsg)           // First: only info and above
    .filterCategory("*.debug=false")   // Second: additional category rules
    .filterDuplicate()                 // Third: suppress duplicates
    .filter("^(?!.*secret).*$")        // Fourth: exclude sensitive data
    .formatPretty()
    .sendToStdErr();

gQtLogger.installMessageHandler();
```

### Different Filters Per Pipeline

```cpp
gQtLogger
    // Console: only warnings and above
    .pipeline()
        .filterLevel(QtWarningMsg)
        .formatPretty(true)
        .sendToStdErr()
    .end()
    
    // Full log file: everything except verbose category
    .pipeline()
        .filterCategory("verbose=false")
        .format("%{time} [%{type}] %{message}")
        .sendToFile("app.log")
    .end()
    
    // Error file: only critical
    .pipeline()
        .filterLevel(QtCriticalMsg)
        .format("%{time} %{file}:%{line} %{message}")
        .sendToFile("errors.log")
    .end();

gQtLogger.installMessageHandler();
```

---

## Navigation

| Previous | Next |
|----------|------|
| [← Formatters](formatters.md) | [Attribute Handlers →](attributes.md) |
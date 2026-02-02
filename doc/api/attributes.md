[QtLogger Docs](../index.md) > [API Reference](index.md) > Attribute Handlers

# Attribute Handlers

Attribute handlers enrich log messages with custom metadata. This section documents all available attribute handler classes.

---

## Table of Contents

- [AttrHandler (Base Class)](#attrhandler-base-class)
- [SeqNumberAttr](#seqnumberattr)
- [AppInfoAttrs](#appinfoattrs)
- [HostInfoAttrs](#hostinfoattrs)
- [FunctionAttrHandler](#functionattrhandler)

---

## AttrHandler (Base Class)

The abstract base class for all attribute handlers.

### Inheritance

```
Handler
└── AttrHandler
```

### Description

`AttrHandler` provides the interface for adding custom attributes to log messages. All attribute handler implementations must override the `attributes()` method.

### Virtual Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `attributes(const LogMessage &lmsg)` | `QVariantHash` | **Pure virtual.** Return attributes to add |

### Inherited Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `type()` | `HandlerType` | Returns `HandlerType::AttrHandler` |
| `process(LogMessage &lmsg)` | `bool` | Calls `attributes()` and merges them into the message |

### How Attributes Work

1. When `process()` is called, the handler's `attributes()` method is invoked
2. The returned `QVariantHash` is merged into the message's attributes
3. Attributes can be accessed via `lmsg.attribute("name")` or used in patterns with `%{name}`

### Example: Custom Attribute Handler

```cpp
#include "qtlogger.h"

class UserContextAttr : public QtLogger::AttrHandler
{
public:
    QVariantHash attributes(const QtLogger::LogMessage &lmsg) override
    {
        Q_UNUSED(lmsg)
        return QVariantHash{
            {"user_id", getCurrentUserId()},
            {"session_id", getCurrentSessionId()},
            {"request_id", QUuid::createUuid().toString()}
        };
    }
};

// Usage
auto attrHandler = QSharedPointer<UserContextAttr>::create();
gQtLogger << attrHandler;
```

---

## SeqNumberAttr

Adds a sequential message number to each log message.

### Inheritance

```
Handler
└── AttrHandler
    └── SeqNumberAttr
```

### Description

`SeqNumberAttr` assigns a unique, incrementing number to each log message. This is useful for tracking message order, especially in asynchronous or multi-threaded environments.

### Constructor

```cpp
explicit SeqNumberAttr(const QString &name = QStringLiteral("seq_number"));
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `name` | `QString` | `"seq_number"` | Attribute name for the sequence number |

### Attribute Added

| Name | Type | Description |
|------|------|-------------|
| `seq_number` (or custom) | `int` | Incrementing counter starting from 1 |

### SimplePipeline Method

```cpp
SimplePipeline &addSeqNumber(const QString &name = QStringLiteral("seq_number"));
```

### Example

```cpp
#include "qtlogger.h"

gQtLogger
    .addSeqNumber()
    .format("#%{seq_number:0>6} %{time hh:mm:ss} %{message}")
    .sendToStdErr();

gQtLogger.installMessageHandler();

qDebug() << "First message";   // #000001 14:30:45 First message
qDebug() << "Second message";  // #000002 14:30:45 Second message
qDebug() << "Third message";   // #000003 14:30:46 Third message
```

### Custom Attribute Name

```cpp
// Use a custom name
gQtLogger
    .addSeqNumber("msg_id")
    .format("[%{msg_id}] %{message}")
    .sendToStdErr();

// Output: [1] Hello
// Output: [2] World
```

### Use Cases

- **Log correlation**: Track message order across multiple outputs
- **Debugging**: Identify missing or out-of-order messages
- **Testing**: Verify all expected messages were logged

---

## AppInfoAttrs

Adds application metadata to log messages.

### Inheritance

```
Handler
└── AttrHandler
    └── AppInfoAttrs
```

### Description

`AppInfoAttrs` adds information about the running application, gathered from `QCoreApplication`. The attributes are captured once at construction time.

### Constructor

```cpp
AppInfoAttrs();
```

### Attributes Added

| Name | Type | Description | Source |
|------|------|-------------|--------|
| `appname` | `QString` | Application name | `QCoreApplication::applicationName()` |
| `appversion` | `QString` | Application version | `QCoreApplication::applicationVersion()` |
| `appdir` | `QString` | Application directory path | `QCoreApplication::applicationDirPath()` |
| `apppath` | `QString` | Full application executable path | `QCoreApplication::applicationFilePath()` |
| `pid` | `qint64` | Process ID | `QCoreApplication::applicationPid()` |

### SimplePipeline Method

```cpp
SimplePipeline &addAppInfo();
```

### Example

```cpp
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("MyApp");
    app.setApplicationVersion("1.2.3");
    
    gQtLogger
        .addAppInfo()
        .formatToJson()
        .sendToFile("app.log");
    
    gQtLogger.installMessageHandler();
    
    qInfo() << "Application started";
    
    return app.exec();
}

// JSON output includes:
// {
//     "appname": "MyApp",
//     "appversion": "1.2.3",
//     "appdir": "/usr/bin",
//     "apppath": "/usr/bin/myapp",
//     "pid": 12345,
//     "message": "Application started",
//     ...
// }
```

### Using in Patterns

```cpp
gQtLogger
    .addAppInfo()
    .format("[%{appname} v%{appversion}] (PID: %{pid}) %{message}")
    .sendToStdErr();

// Output: [MyApp v1.2.3] (PID: 12345) Application started
```

### Use Cases

- **Centralized logging**: Identify which application produced a log entry
- **Multi-process systems**: Distinguish logs from different processes
- **Debugging**: Include version information in error reports

---

## HostInfoAttrs

Adds host machine information to log messages.

> **Note**: Requires `QTLOGGER_NETWORK` to be defined.

### Inheritance

```
Handler
└── AttrHandler
    └── HostInfoAttrs
```

### Description

`HostInfoAttrs` adds information about the host machine, including hostname and IP addresses. The attributes are captured once at construction time.

### Constructor

```cpp
HostInfoAttrs();
```

### Attributes Added

| Name | Type | Description | Source |
|------|------|-------------|--------|
| `hostname` | `QString` | Local hostname | `QHostInfo::localHostName()` |
| `hostaddress` | `QString` | Primary IP address | First non-loopback IPv4 address |

### SimplePipeline Method

```cpp
SimplePipeline &addHostInfo();
```

> **Note**: Only available when `QTLOGGER_NETWORK` is defined.

### Example

```cpp
#include "qtlogger.h"

gQtLogger
    .addAppInfo()
    .addHostInfo()
    .formatToJson(true)
    .sendToHttp("https://logs.example.com/ingest");

gQtLogger.installMessageHandler();

qInfo() << "Service started";

// JSON output includes:
// {"hostname":"server01","hostaddress":"192.168.1.100","appname":"MyService",...}
```

### Using in Patterns

```cpp
gQtLogger
    .addHostInfo()
    .format("[%{hostname}/%{hostaddress}] %{message}")
    .sendToFile("distributed.log");

// Output: [server01/192.168.1.100] Service started
```

### Use Cases

- **Distributed systems**: Identify which server produced a log entry
- **Cloud environments**: Track container/instance origins
- **Network debugging**: Include IP address for correlation

---

## FunctionAttrHandler

An attribute handler that uses a custom function.

### Inheritance

```
Handler
└── AttrHandler
    └── FunctionAttrHandler
```

### Type Alias

```cpp
using Function = std::function<QVariantHash(const LogMessage &lmsg)>;
```

### Constructor

```cpp
FunctionAttrHandler(const Function &function);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `function` | `Function` | Function returning attributes hash |

### SimplePipeline Method

```cpp
SimplePipeline &attrHandler(std::function<QVariantHash(const LogMessage &lmsg)> func);
```

### Example

```cpp
#include "qtlogger.h"

// Add custom attributes via lambda
gQtLogger
    .attrHandler([](const QtLogger::LogMessage &lmsg) {
        return QVariantHash{
            {"env", qEnvironmentVariable("APP_ENV", "development")},
            {"build", QString(__DATE__) + " " + __TIME__}
        };
    })
    .format("[%{env}] %{message} (build: %{build})")
    .sendToStdErr();

gQtLogger.installMessageHandler();

// Output: [production] Application started (build: Jan 15 2024 14:30:45)
```

### Dynamic Attributes

Unlike `AppInfoAttrs` and `HostInfoAttrs` which capture values at construction, `FunctionAttrHandler` is called for each message, allowing dynamic values:

```cpp
// Thread-local context
thread_local QString currentOperation;

gQtLogger
    .attrHandler([](const QtLogger::LogMessage &) {
        return QVariantHash{
            {"operation", currentOperation},
            {"timestamp_ns", QDateTime::currentMSecsSinceEpoch() * 1000000}
        };
    })
    .format("[%{operation?9}] %{message}")
    .sendToStdErr();

// In your code:
currentOperation = "database";
qDebug() << "Query executed";  // [database] Query executed

currentOperation = "network";
qDebug() << "Request sent";    // [network ] Request sent
```

### Request Context Example

```cpp
// Request tracking in a web service
class RequestContext
{
public:
    static thread_local QString requestId;
    static thread_local QString userId;
    static thread_local QString clientIp;
};

gQtLogger
    .attrHandler([](const QtLogger::LogMessage &) {
        return QVariantHash{
            {"request_id", RequestContext::requestId},
            {"user_id", RequestContext::userId},
            {"client_ip", RequestContext::clientIp}
        };
    })
    .formatToJson()
    .sendToFile("requests.log");

// When handling a request:
RequestContext::requestId = QUuid::createUuid().toString();
RequestContext::userId = authenticatedUser;
RequestContext::clientIp = request.clientAddress();

qInfo() << "Processing request";
// All log messages now include request context
```

### Combining Multiple Attribute Handlers

```cpp
gQtLogger
    .addSeqNumber()
    .addAppInfo()
    .addHostInfo()
    .attrHandler([](const QtLogger::LogMessage &lmsg) {
        // Add computed attributes
        return QVariantHash{
            {"severity", lmsg.type() >= QtWarningMsg ? "HIGH" : "LOW"},
            {"source_module", extractModule(lmsg.file())}
        };
    })
    .formatToJson()
    .sendToHttp("https://logs.example.com");
```

---

## Working with Attributes

### Accessing Attributes in Code

```cpp
// In a custom handler
bool process(LogMessage &lmsg) override
{
    // Read an attribute
    if (lmsg.hasAttribute("user_id")) {
        QString userId = lmsg.attribute("user_id").toString();
        // ...
    }
    
    // Modify an attribute
    lmsg.setAttribute("processed", true);
    
    // Remove an attribute
    lmsg.removeAttribute("temporary");
    
    // Get all attributes
    QVariantHash attrs = lmsg.attributes();
    
    // Get all attributes including built-in ones
    QVariantHash allAttrs = lmsg.allAttributes();
    
    return true;
}
```

### Using Attributes in Patterns

All attributes can be used in `PatternFormatter` patterns:

```cpp
gQtLogger
    .addSeqNumber()
    .addAppInfo()
    .attrHandler([](const QtLogger::LogMessage &) {
        return QVariantHash{{"custom", "value"}};
    })
    .format("%{seq_number} [%{appname}] %{custom}: %{message}")
    .sendToStdErr();
```

### Optional Attributes in Patterns

Use the `?` syntax for attributes that might not be present:

```cpp
// %{attr?}    - Empty string if missing
// %{attr?N}   - Remove N chars before if missing
// %{attr?N,M} - Remove N chars before and M chars after if missing

gQtLogger
    .format("[%{user_id?}] %{message}")  // Shows "[] message" if no user_id
    .sendToStdErr();

gQtLogger
    .format("User: %{user_id?6} | %{message}")  // Removes "User: " if no user_id
    .sendToStdErr();
```

### Attributes in JSON Output

All attributes are automatically included in JSON output:

```cpp
gQtLogger
    .addSeqNumber()
    .addAppInfo()
    .formatToJson()
    .sendToFile("app.log");

// Output:
// {
//     "type": "info",
//     "time": "2024-01-15T14:30:45.123",
//     "message": "Hello",
//     "seq_number": 1,
//     "appname": "MyApp",
//     "appversion": "1.0.0",
//     "pid": 12345,
//     ...
// }
```

---

## Navigation

| Previous | Next |
|----------|------|
| [← Filters](filters.md) | [Advanced Usage →](../advanced.md) |
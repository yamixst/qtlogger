[QtLogger Docs](../index.md) > [API Reference](index.md) > Attribute Handlers

# Attribute Handlers

Attribute handlers enrich log messages with custom metadata. This section documents all available attribute handler classes.

---

## Table of Contents

- [AttrHandler (Base Class)](#attrhandler-base-class)
- [SeqNumberAttr](#seqnumberattr)
- [AppInfoAttrs](#appinfoattrs)
- [AppUuidAttr](#appuuidattr)
- [SysInfoAttrs](#sysinfoattrs)
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

## AppUuidAttr

Adds a persistent application UUID to log messages.

### Inheritance

```
Handler
└── AttrHandler
    └── AppUuidAttr
```

### Description

`AppUuidAttr` generates a unique UUID for the application instance and stores it in `QSettings` (UserScope). The UUID is created once on first use and persists across application restarts. This is useful for tracking logs from specific application installations.

### Constructor

```cpp
explicit AppUuidAttr(const QString &name = QStringLiteral("app_uuid"));
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `name` | `QString` | `"app_uuid"` | Attribute name for the UUID |

### Attributes Added

| Name | Type | Description |
|------|------|-------------|
| `app_uuid` (or custom) | `QString` | Persistent UUID without braces (e.g., `"550e8400-e29b-41d4-a716-446655440000"`) |

### Storage

The UUID is stored in `QSettings` with:
- **Scope**: `QSettings::UserScope`
- **Organization**: `QCoreApplication::organizationName()`
- **Application**: `QCoreApplication::applicationName()`
- **Key**: `"app_uuid"`

### SimplePipeline Method

```cpp
SimplePipeline &addAppUuid(const QString &name = QStringLiteral("app_uuid"));
```

### Example

```cpp
#include "qtlogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("MyCompany");
    app.setApplicationName("MyApp");
    
    gQtLogger
        .addAppUuid()
        .formatToJson()
        .sendToFile("app.log");
    
    gQtLogger.installMessageHandler();
    
    qInfo() << "Application started";
    
    return app.exec();
}

// JSON output includes:
// {
//     "app_uuid": "550e8400-e29b-41d4-a716-446655440000",
//     "message": "Application started",
//     ...
// }
```

### Using in Patterns

```cpp
gQtLogger
    .addAppUuid()
    .format("[%{app_uuid}] %{message}")
    .sendToStdErr();

// Output: [550e8400-e29b-41d4-a716-446655440000] Application started
```

### Custom Attribute Name

```cpp
gQtLogger
    .addAppUuid("installation_id")
    .format("[%{installation_id}] %{message}")
    .sendToStdErr();
```

### Persistence Behavior

```cpp
// First run - UUID is generated and stored
gQtLogger.addAppUuid();
qInfo() << "First run";  // [abc-123-...] First run

// Second run (after restart) - same UUID is loaded from QSettings
gQtLogger.addAppUuid();
qInfo() << "Second run"; // [abc-123-...] Second run (same UUID)
```

### Use Cases

- **Installation tracking**: Identify specific application installations in centralized logs
- **Sentry integration**: Track errors from specific installations
- **Analytics**: Correlate logs from the same installation over time
- **Multi-instance debugging**: Distinguish between multiple installations on the same machine

### Combining with Other Attributes

```cpp
gQtLogger
    .addAppUuid()
    .addAppInfo()
    .addHostInfo()
    .formatToJson()
    .sendToHttp("https://logs.example.com/ingest");

// Each log entry includes installation UUID, app info, and host info
```

---

## SysInfoAttrs

Adds system and OS information to log messages.

### Inheritance

```
Handler
└── AttrHandler
    └── SysInfoAttrs
```

### Description

`SysInfoAttrs` adds information about the operating system, kernel, and CPU architecture. The attributes are captured once at construction time and cached for efficiency.

### Constructor

```cpp
SysInfoAttrs();
```

### Static Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `instance()` | `SysInfoAttrsPtr` | Get cached singleton instance |

### Attributes Added

| Name | Type | Description | Source |
|------|------|-------------|--------|
| `os_name` | `QString` | Operating system type | `QSysInfo::productType()` |
| `os_version` | `QString` | Operating system version | `QSysInfo::productVersion()` |
| `kernel_type` | `QString` | Kernel type (e.g., "linux", "darwin") | `QSysInfo::kernelType()` |
| `kernel_version` | `QString` | Kernel version string | `QSysInfo::kernelVersion()` |
| `cpu_arch` | `QString` | Current CPU architecture | `QSysInfo::currentCpuArchitecture()` |
| `build_abi` | `QString` | Build ABI string | `QSysInfo::buildAbi()` |
| `build_cpu_arch` | `QString` | Build-time CPU architecture | `QSysInfo::buildCpuArchitecture()` |
| `pretty_product_name` | `QString` | Human-readable OS name | `QSysInfo::prettyProductName()` |

### SimplePipeline Method

```cpp
SimplePipeline &addSysInfo();
```

### Example

```cpp
#include "qtlogger.h"

gQtLogger
    .addSysInfo()
    .formatToJson()
    .sendToFile("system.log");

gQtLogger.installMessageHandler();

qInfo() << "System initialized";

// JSON output includes:
// {
//     "os_name": "ubuntu",
//     "os_version": "22.04",
//     "kernel_type": "linux",
//     "kernel_version": "5.15.0-91-generic",
//     "cpu_arch": "x86_64",
//     "build_abi": "x86_64-little_endian-lp64",
//     "pretty_product_name": "Ubuntu 22.04.3 LTS",
//     "message": "System initialized",
//     ...
// }
```

### Using in Patterns

```cpp
gQtLogger
    .addSysInfo()
    .format("[%{os_name} %{os_version}] [%{cpu_arch}] %{message}")
    .sendToStdErr();

// Output: [ubuntu 22.04] [x86_64] Application started
```

### Use Cases

- **Sentry integration**: Provides OS context for error tracking
- **Diagnostics**: Include system info in crash reports
- **Multi-platform support**: Track which platforms encounter issues
- **Centralized logging**: Identify host OS in aggregated logs

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
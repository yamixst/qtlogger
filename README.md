![Project Status](https://img.shields.io/badge/status-experimental-orange)

# Qt Logger

A simple yet powerful logging solution for the Qt Framework. This project is designed to provide developers with an intuitive and configurable logging system for Qt-based applications. QtLogger features a customizable pipeline for processing log messages through the `qInstallMessageHandler()` function, allowing for flexible handling and output of logs.

## Quick start

Add qtlogger.h to your project and use the global gQtLogger object for configuration:

```cpp

#include "qtlogger.h"

int main(int argc, char *argv[])
{
    gQtLogger->configure();

    qDebug() << "Hello QtLogger!";

    return 0;
}

```

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

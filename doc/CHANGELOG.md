# Changelog

All notable changes to QtLogger will be documented in this file.

## [0.8.0]

### Added

- `RotatingFileSink` with configurable rotation modes (by size, by time)
- `PrettyFormatter` with colorized output support
- Compact JSON formatting support

### Changed

- Refactored configuration API with `path` and rotating sink support
- Improved `OwnThreadHandler` thread safety with mutex guard
- Updated README with library comparison table and new examples

## [0.7.1]

### Added

- `demo_showcase` example demonstrating QtLogger features

### Changed

- Refactored `OwnThreadHandler` thread/worker management for improved reliability
- Updated README with `moveToOwnThread()` usage examples
- Improved test suite to use `resetOwnThread()` in `OwnThreadHandler` tests

## [0.7.0]

### Added

- Colored console output support for `StdOutSink` and `StdErrSink`
- Project documentation (`ARCHITECTURE.md`, `FEATURES.md`)
- Version bump script

### Changed

- Bumped minimum requirement to C++17
- Updated project status to beta

## [0.6.0]

### Added

- `LevelFilter` for filtering messages by severity level
- Extended `PatternFormatter` with custom attributes, fixed-width formatting, truncation, and new placeholders (`%{shortfile}`, `%{func}`, `%{qthreadptr}`)
- `FunctionHandler` and `FunctionAttrHandler` for custom processing
- Steady clock timestamp in `LogMessage`
- CMake build system with CTest integration
- Comprehensive unit test suite
- New examples: `config_from_file`, `simple_config`

### Changed

- Renamed `OwnThreadPipeline` to `OwnThreadHandler`
- Renamed `TypedPipeline` to `SortedPipeline`
- Improved API: `operator<<` for pipeline configuration, virtual `flush()`, configurable sequence number name
- Better cross-platform compatibility and symbol export with `QTLOGGER_DECL_SPEC`

### Fixed

- Pattern formatter operator detection and formatting issues
- Thread ID type consistency
- Qt 5.14+ compatibility

cmake_minimum_required(VERSION 3.16)

project(test_formatters LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

# Create test executable
add_executable(test_formatters
    test_formatters.cpp
    mock_logmessage.h
)

# Create mock test executable
add_executable(test_formatters_mocks
    test_formatters_mocks.cpp
    mock_logmessage.h
)

# Create pattern formatter test executable
add_executable(test_patternformatter
    test_patternformatter.cpp
    mock_logmessage.h
)

target_link_libraries(test_formatters
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_link_libraries(test_formatters_mocks
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_link_libraries(test_patternformatter
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_include_directories(test_formatters PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_include_directories(test_formatters_mocks PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_include_directories(test_patternformatter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# Add tests to CTest
add_test(NAME FormattersTest COMMAND test_formatters)
add_test(NAME FormattersMocksTest COMMAND test_formatters_mocks)
add_test(NAME PatternFormatterTest COMMAND test_patternformatter)

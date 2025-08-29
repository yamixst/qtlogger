cmake_minimum_required(VERSION 3.16)

project(test_qtlogger_header LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

# Create test executable
add_executable(test_qtlogger_header
    test_qtlogger_header.cpp
)

target_link_libraries(test_qtlogger_header
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

# Include the generated qtlogger.h from the project root
target_include_directories(test_qtlogger_header PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# Add test to CTest
add_test(NAME QtLoggerHeaderTest COMMAND test_qtlogger_header)

cmake_minimum_required(VERSION 3.16)

project(test_logger LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test Concurrent)

# Create test executable
add_executable(test_logger
    test_logger.cpp
    mock_sink.h
    mock_handler.h
)

target_link_libraries(test_logger
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Concurrent
    qtlogger
)

target_include_directories(test_logger PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# Add test to CTest
add_test(NAME LoggerTest COMMAND test_logger)

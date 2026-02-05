# Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.14)

project(sentry_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

# Enable network support for QtLogger
add_definitions(-DQTLOGGER_NETWORK)

add_executable(sentry_example main.cpp)

target_include_directories(sentry_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../..)

target_link_libraries(sentry_example
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    qtlogger
)

cmake_minimum_required(VERSION 3.16)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtLoggerVersion.cmake")

project(qtlogger VERSION ${QTLOGGER_PROJECT_VERSION} LANGUAGES CXX)

option(QTLOGGER_NO_EXAMPLES "Disable building examples" OFF)
option(QTLOGGER_NO_TESTS "Disable building tests" OFF)

option(QTLOGGER_LIBRARY "Build qtlogger as shared library" OFF)
option(QTLOGGER_DEBUG_OUTPUT "Enable qtlogger debug output" OFF)
option(QTLOGGER_NO_THREAD "Disable qtlogger threading support" OFF)
option(QTLOGGER_NETWORK "Enable qtlogger network support" OFF)
option(QTLOGGER_JOURNAL "Enable qtlogger systemd journal support" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
set(QT_COMPONENTS Core)

if(NOT QTLOGGER_NO_EXAMPLES)
    list(APPEND QT_COMPONENTS Concurrent)
endif()

if(NOT QTLOGGER_NO_TESTS)
    list(APPEND QT_COMPONENTS Test)
endif()

if(QTLOGGER_NETWORK)
    list(APPEND QT_COMPONENTS Network)
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMPONENTS})

add_subdirectory(src/qtlogger)

if(NOT QTLOGGER_NO_EXAMPLES)
    add_subdirectory(examples)
endif()

if(NOT QTLOGGER_NO_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_NAME "qtlogger")
set(CPACK_PACKAGE_VENDOR "Mikhail Yatsenko")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced Qt logging library")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_CONTACT "mikhail.yatsenko@gmail.com")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/yamixst/qtlogger")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)

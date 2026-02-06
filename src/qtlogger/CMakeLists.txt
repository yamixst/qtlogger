cmake_minimum_required(VERSION 3.16)

project(qtlogger LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(QTLOGGER_LIBRARY "Build as shared library" OFF)
option(QTLOGGER_DEBUG_OUTPUT "Enable debug output" OFF)
option(QTLOGGER_NO_THREAD "Disable threading support" OFF)
option(QTLOGGER_NETWORK "Enable network support" OFF)
option(QTLOGGER_JOURNAL "Enable systemd journal support" OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
set(QT_COMPONENTS Core)

if(QTLOGGER_NETWORK)
    list(APPEND QT_COMPONENTS Network)
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMPONENTS})

set(QTLOGGER_SOURCES
    attrhandlers/appinfoattrs.cpp
    attrhandlers/seqnumberattr.cpp
    attrhandlers/sysinfoattrs.cpp
    configure.cpp
    filters/categoryfilter.cpp
    filters/duplicatefilter.cpp
    filters/regexpfilter.cpp
    formatters/jsonformatter.cpp
    formatters/patternformatter.cpp
    formatters/prettyformatter.cpp
    formatters/sentryformatter.cpp
    logger.cpp
    pipeline.cpp
    simplepipeline.cpp
    sinks/coloredconsole.cpp
    sinks/filesink.cpp
    sinks/iodevicesink.cpp
    sinks/rotatingfilesink.cpp
    sinks/signalsink.cpp
    sinks/stderrsink.cpp
    sinks/stdoutsink.cpp
    sortedpipeline.cpp
    utils.cpp
)

set(QTLOGGER_HEADERS
    attrhandler.h
    attrhandlers/appinfoattrs.h
    attrhandlers/functionattrhandler.h
    attrhandlers/seqnumberattr.h
    attrhandlers/sysinfoattrs.h
    configure.h
    filter.h
    filters/categoryfilter.h
    filters/duplicatefilter.h
    filters/functionfilter.h
    filters/levelfilter.h
    filters/regexpfilter.h
    formatter.h
    formatters/functionformatter.h
    formatters/jsonformatter.h
    formatters/patternformatter.h
    formatters/prettyformatter.h
    formatters/sentryformatter.h
    formatters/qtlogmessageformatter.h
    functionhandler.h
    handler.h
    logger.h
    logger_global.h
    logmessage.h
    messagepatterns.h
    pipeline.h
    qtlogger.h
    simplepipeline.h
    sink.h
    sinks/coloredconsole.h
    sinks/filesink.h
    sinks/iodevicesink.h
    sinks/platformstdsink.h
    sinks/rotatingfilesink.h
    sinks/signalsink.h
    sinks/stderrsink.h
    sinks/stdoutsink.h
    sortedpipeline.h
    utils.h
    version.h
)

if(NOT QTLOGGER_NO_THREAD)
    list(APPEND QTLOGGER_HEADERS ownthreadhandler.h)
endif()

if(QTLOGGER_NETWORK)
    list(APPEND QTLOGGER_SOURCES
        attrhandlers/hostinfoattrs.cpp
        sinks/httpsink.cpp
    )
    list(APPEND QTLOGGER_HEADERS
        attrhandlers/hostinfoattrs.h
        sinks/httpsink.h
    )
endif()

if(WIN32)
    list(APPEND QTLOGGER_SOURCES sinks/windebugsink.cpp)
    list(APPEND QTLOGGER_HEADERS sinks/windebugsink.h)
endif()

if(APPLE)
    list(APPEND QTLOGGER_SOURCES sinks/oslogsink.cpp)
    list(APPEND QTLOGGER_HEADERS sinks/oslogsink.h)
endif()

if(ANDROID)
    list(APPEND QTLOGGER_SOURCES sinks/androidlogsink.cpp)
    list(APPEND QTLOGGER_HEADERS sinks/androidlogsink.h)
elseif(UNIX AND NOT APPLE)
    list(APPEND QTLOGGER_SOURCES sinks/syslogsink.cpp)
    list(APPEND QTLOGGER_HEADERS sinks/syslogsink.h)
endif()

if(QTLOGGER_JOURNAL)
    list(APPEND QTLOGGER_SOURCES sinks/sdjournalsink.cpp)
    list(APPEND QTLOGGER_HEADERS sinks/sdjournalsink.h)
endif()

if(QTLOGGER_LIBRARY)
    add_library(qtlogger SHARED ${QTLOGGER_SOURCES} ${QTLOGGER_HEADERS})
else()
    add_library(qtlogger STATIC ${QTLOGGER_SOURCES} ${QTLOGGER_HEADERS})
endif()

target_link_libraries(qtlogger PRIVATE Qt${QT_VERSION_MAJOR}::Core)
if(QTLOGGER_NETWORK)
    target_link_libraries(qtlogger PRIVATE Qt${QT_VERSION_MAJOR}::Network)
endif()

target_compile_definitions(qtlogger PRIVATE QTLOGGER_LIBRARY)

if(NOT QTLOGGER_LIBRARY)
    target_compile_definitions(qtlogger PUBLIC QTLOGGER_STATIC)
endif()

if(QTLOGGER_DEBUG_OUTPUT)
    target_compile_definitions(qtlogger PRIVATE QTLOGGER_DEBUG)
endif()

if(QTLOGGER_NO_THREAD)
    target_compile_definitions(qtlogger PRIVATE QTLOGGER_NO_THREAD)
endif()

if(QTLOGGER_NETWORK)
    target_compile_definitions(qtlogger PRIVATE QTLOGGER_NETWORK)
endif()

if(APPLE)
    target_compile_definitions(qtlogger PRIVATE QTLOGGER_OSLOG)
endif()

if(ANDROID)
    target_compile_definitions(qtlogger PRIVATE QTLOGGER_ANDROIDLOG)
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_compile_definitions(qtlogger PRIVATE QTLOGGER_SYSLOG)
endif()

if(QTLOGGER_JOURNAL)
    target_compile_definitions(qtlogger PRIVATE QTLOGGER_SDJOURNAL)
    # Find and link libsystemd
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SYSTEMD libsystemd)
        if(SYSTEMD_FOUND)
            target_link_libraries(qtlogger PRIVATE ${SYSTEMD_LIBRARIES})
            target_include_directories(qtlogger PRIVATE ${SYSTEMD_INCLUDE_DIRS})
        endif()
    endif()
endif()

target_include_directories(qtlogger
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

CONFIG += c++14

!qtlogger_library {
    DEFINES *= QTLOGGER_STATIC
}

qtlogger_debug_output {
    DEFINES *= QTLOGGER_DEBUG
}

qtlogger_no_thread {
    DEFINES *= QTLOGGER_NO_THREAD
}

qtlogger_network {
    DEFINES *= QTLOGGER_NETWORK
    QT *= network
    SOURCES += $$PWD/sinks/httpsink.cpp
    HEADERS += $$PWD/sinks/httpsink.h
}

ios {
    DEFINES *= QTLOGGER_IOSLOG
    SOURCES += $$PWD/sinks/ioslogsink.cpp
    HEADERS += $$PWD/sinks/ioslogsink.h
}

linux:android {
    DEFINES *= QTLOGGER_ANDROIDLOG
    SOURCES += $$PWD/sinks/androidlogsink.cpp
    HEADERS += $$PWD/sinks/androidlogsink.h
}

unix:!android {
    DEFINES *= QTLOGGER_SYSLOG
    SOURCES += $$PWD/sinks/syslogsink.cpp
    HEADERS += $$PWD/sinks/syslogsink.h
}

qtlogger_journal {
    DEFINES *= QTLOGGER_JOURNAL
    SOURCES += $$PWD/sinks/journalsink.cpp
    HEADERS += $$PWD/sinks/journalsink.h
}

SOURCES += \
    $$PWD/filters/regexpfilter.cpp \
    $$PWD/formatters/jsonformatter.cpp \
    $$PWD/formatters/patternformatter.cpp \
    $$PWD/formatters/prettyformatter.cpp \
    $$PWD/logger.cpp \
    $$PWD/pipelinehandler.cpp \
    $$PWD/setmessagepattern.cpp \
    $$PWD/sinks/filesink.cpp \
    $$PWD/sinks/iodevicesink.cpp \
    $$PWD/sinks/rotatingfilesink.cpp \
    $$PWD/sinks/signalsink.cpp \
    $$PWD/sinks/stderrsink.cpp \
    $$PWD/sinks/stdoutsink.cpp

HEADERS += \
    $$PWD/filter.h \
    $$PWD/filters/functionfilter.h \
    $$PWD/filters/regexpfilter.h \
    $$PWD/formatter.h \
    $$PWD/formatters/defaultformatter.h \
    $$PWD/formatters/functionformatter.h \
    $$PWD/formatters/jsonformatter.h \
    $$PWD/formatters/nullformatter.h \
    $$PWD/formatters/patternformatter.h \
    $$PWD/formatters/prettyformatter.h \
    $$PWD/formatters/qtlogmessageformatter.h \
    $$PWD/logger.h \
    $$PWD/logger_global.h \
    $$PWD/logmessage.h \
    $$PWD/messagehandler.h \
    $$PWD/messagepatterns.h \
    $$PWD/pipelinehandler.h \
    $$PWD/setmessagepattern.h \
    $$PWD/sink.h \
    $$PWD/sinks/filesink.h \
    $$PWD/sinks/iodevicesink.h \
    $$PWD/sinks/rotatingfilesink.h \
    $$PWD/sinks/signalsink.h \
    $$PWD/sinks/stderrsink.h \
    $$PWD/sinks/stdlogsink.h \
    $$PWD/sinks/stdoutsink.h \
    $$PWD/version.h


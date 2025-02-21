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
else {
    HEADERS += \
    $$PWD/ownthreadhandler.h
}

qtlogger_network {
    DEFINES *= QTLOGGER_NETWORK
    QT *= network
    SOURCES += \
        $$PWD/attrhandlers/hostinfoattrs.cpp \
        $$PWD/sinks/httpsink.cpp
    HEADERS += \
        $$PWD/attrhandlers/hostinfoattrs.h \
        $$PWD/sinks/httpsink.h
}

windows {
    SOURCES += $$PWD/sinks/windebugsink.cpp
    HEADERS += $$PWD/sinks/windebugsink.h
}

macos | ios {
    DEFINES *= QTLOGGER_OSLOG
    SOURCES += $$PWD/sinks/oslogsink.cpp
    HEADERS += $$PWD/sinks/oslogsink.h
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
    DEFINES *= QTLOGGER_SDJOURNAL
    SOURCES += $$PWD/sinks/sdjournalsink.cpp
    HEADERS += $$PWD/sinks/sdjournalsink.h
}

SOURCES += \
    $$PWD/attrhandlers/appinfoattrs.cpp \
    $$PWD/attrhandlers/seqnumberattr.cpp \
    $$PWD/configure.cpp \
    $$PWD/filters/categoryfilter.cpp \
    $$PWD/filters/duplicatefilter.cpp \
    $$PWD/filters/regexpfilter.cpp \
    $$PWD/formatters/jsonformatter.cpp \
    $$PWD/formatters/patternformatter.cpp \
    $$PWD/formatters/prettyformatter.cpp \
    $$PWD/logger.cpp \
    $$PWD/pipeline.cpp \
    $$PWD/setmessagepattern.cpp \
    $$PWD/simplepipeline.cpp \
    $$PWD/sinks/filesink.cpp \
    $$PWD/sinks/iodevicesink.cpp \
    $$PWD/sinks/rotatingfilesink.cpp \
    $$PWD/sinks/signalsink.cpp \
    $$PWD/sinks/stderrsink.cpp \
    $$PWD/sinks/stdoutsink.cpp \
    $$PWD/sortedpipeline.cpp

HEADERS += \
    $$PWD/attrhandler.h \
    $$PWD/attrhandlers/appinfoattrs.h \
    $$PWD/attrhandlers/seqnumberattr.h \
    $$PWD/configure.h \
    $$PWD/filter.h \
    $$PWD/filters/categoryfilter.h \
    $$PWD/filters/duplicatefilter.h \
    $$PWD/filters/functionfilter.h \
    $$PWD/filters/regexpfilter.h \
    $$PWD/formatter.h \
    $$PWD/formatters/functionformatter.h \
    $$PWD/formatters/jsonformatter.h \
    $$PWD/formatters/patternformatter.h \
    $$PWD/formatters/prettyformatter.h \
    $$PWD/formatters/qtlogmessageformatter.h \
    $$PWD/handler.h \
    $$PWD/logger.h \
    $$PWD/logger_global.h \
    $$PWD/logmessage.h \
    $$PWD/messagepatterns.h \
    $$PWD/pipeline.h \
    $$PWD/setmessagepattern.h \
    $$PWD/simplepipeline.h \
    $$PWD/sink.h \
    $$PWD/sinks/filesink.h \
    $$PWD/sinks/iodevicesink.h \
    $$PWD/sinks/platformstdsink.h \
    $$PWD/sinks/rotatingfilesink.h \
    $$PWD/sinks/signalsink.h \
    $$PWD/sinks/stderrsink.h \
    $$PWD/sinks/stdoutsink.h \
    $$PWD/sortedpipeline.h \
    $$PWD/version.h

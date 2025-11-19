# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

# Usage example
#
# app.pro:
# SUBDIRS += 3rdparty/qtlogger
#
# src/src.pro:
# include(../3rdparty/qtlogger/qtlogger_link.pri)

CONFIG += c++17

qtlogger_static {
    DEFINES *= QTLOGGER_STATIC
}

qtlogger_no_thread {
    DEFINES *= QTLOGGER_NO_THREAD
}

qtlogger_network {
    DEFINES *= QTLOGGER_NETWORK
    QT *= network
}

macos | ios {
    DEFINES *= QTLOGGER_OSLOG
}

linux:android {
    DEFINES *= QTLOGGER_ANDROIDLOG
}

unix:!android {
    DEFINES *= QTLOGGER_SYSLOG
}

qtlogger_journal {
    DEFINES *= QTLOGGER_SDJOURNAL
}

isEmpty(QTLOGGER_TARGET) {
    QTLOGGER_TARGET = qtlogger
}

isEmpty(QTLOGGER_TARGET_FILE_NAME) {
    qtlogger_static {
        QTLOGGER_TARGET_FILE_NAME = $${QMAKE_PREFIX_STATICLIB}$${QTLOGGER_TARGET}.$${QMAKE_EXTENSION_STATICLIB}
    }
    else {
        QTLOGGER_TARGET_FILE_NAME = $${QMAKE_PREFIX_SHLIB}$${QTLOGGER_TARGET}.$${QMAKE_EXTENSION_SHLIB}
    }
}

isEmpty(QTLOGGER_OUT_PWD) {
    QTLOGGER_OUT_PWD = $$clean_path($$OUT_PWD/$$relative_path($$PWD/src/qtlogger, $$_PRO_FILE_PWD_))
}
isEmpty(QTLOGGER_OUT_FILE) {
    QTLOGGER_OUT_FILE = $$QTLOGGER_OUT_PWD/$$QTLOGGER_TARGET_FILE_NAME
}

INCLUDEPATH += $$PWD/include

PRE_TARGETDEPS += $$QTLOGGER_OUT_FILE

LIBS += -L$$QTLOGGER_OUT_PWD -l$$QTLOGGER_TARGET

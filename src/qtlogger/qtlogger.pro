# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

TEMPLATE = lib

!isEmpty(QTLOGGER_TARGET) {
    TARGET = $$QTLOGGER_TARGET
}
else {
    TARGET = qtlogger
}

QT -= gui

CONFIG -= debug_and_release
CONFIG *= warn_on

# CONFIG += qtlogger_static
# CONFIG += qtlogger_debug_output
# CONFIG += qtlogger_no_thread
# CONFIG += qtlogger_network
# CONFIG += qtlogger_journal

qtlogger_static {
    CONFIG += staticlib
    DEFINES *= QTLOGGER_STATIC
} else {
    CONFIG += qtlogger_library
    DEFINES *= QTLOGGER_LIBRARY
}

include($$PWD/qtlogger.pri)

unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

TEMPLATE = app

TARGET = simple

CONFIG += qt
QT -= gui
QT += core network

include(../../qtlogger_link.pri)

# HEADERS += $$PWD/../../qtlogger.h

INCLUDEPATH += $$PWD/../../src

SOURCES += $$PWD/main.cpp

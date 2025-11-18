# Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT

TEMPLATE = app

TARGET = one_line_config

CONFIG += qt
QT -= gui
QT += core network

include(../../qtlogger_link.pri)

INCLUDEPATH += $$PWD/../../src

SOURCES += $$PWD/main.cpp

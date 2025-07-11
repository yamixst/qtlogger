# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

TEMPLATE = app

TARGET = pattern_formatter

CONFIG += qt
QT -= gui
QT += core network

include(../../qtlogger_link.pri)

INCLUDEPATH += $$PWD/../../src

SOURCES += $$PWD/main.cpp

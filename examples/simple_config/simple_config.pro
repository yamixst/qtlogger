# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

TEMPLATE = app

TARGET = simple_config

CONFIG += qt
QT -= gui
QT += core network

include(../../qtlogger_link.pri)

INCLUDEPATH += $$PWD/../../src

SOURCES += $$PWD/main.cpp

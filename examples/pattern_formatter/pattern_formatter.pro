# Copyright (C) 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT

TEMPLATE = app

TARGET = pattern_formatter

CONFIG += qt
QT -= gui
QT += core

include(../../qtlogger_link.pri)

SOURCES += $$PWD/main.cpp

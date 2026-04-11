# Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT

TEMPLATE = app

TARGET = simple_config

CONFIG += qt
QT -= gui

include(../../qtlogger_link.pri)

SOURCES += $$PWD/main.cpp

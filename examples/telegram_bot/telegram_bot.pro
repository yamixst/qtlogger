# Copyright (C) 2026 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT

TEMPLATE = app

TARGET = telegram_bot

CONFIG += qt
QT -= gui

CONFIG += qtlogger_network
include(../../qtlogger_link.pri)

SOURCES += $$PWD/main.cpp

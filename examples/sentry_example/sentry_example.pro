# Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT

TEMPLATE = app

TARGET = sentry_example

CONFIG += qt
QT -= gui
QT += core network

CONFIG += qtlogger_network
include(../../qtlogger_link.pri)

SOURCES += $$PWD/main.cpp

# Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT

QT += core network

CONFIG += c++17 console
CONFIG -= app_bundle

DEFINES += QTLOGGER_NETWORK

TARGET = sentry_example

SOURCES += main.cpp

include(../../qtlogger_link.pri)

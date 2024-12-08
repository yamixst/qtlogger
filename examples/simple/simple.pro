# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

TEMPLATE = app

TARGET = simple

CONFIG += qt
QT -= gui
QT += core network

HEADERS += $$PWD/../../qtlogger.h
SOURCES += $$PWD/main.cpp

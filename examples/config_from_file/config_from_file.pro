# Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT

TEMPLATE = app

TARGET = config_from_file

CONFIG += qt
QT -= gui
QT += core

include(../../qtlogger_link.pri)

SOURCES += $$PWD/main.cpp

CONFIG += file_copies
COPIES += config_file
config_file.files = $$PWD/config.ini
config_file.path = $$OUT_PWD

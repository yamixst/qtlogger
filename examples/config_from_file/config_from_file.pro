QT += core
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = config_from_file
TEMPLATE = app

SOURCES += main.cpp

include(../../qtlogger_link.pri)

CONFIG += file_copies
COPIES += config_file
config_file.files = $$PWD/config.ini
config_file.path = $$OUT_PWD

# Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT

TEMPLATE = subdirs

CONFIG += qtlogger_no_tests
# CONFIG += qtlogger_no_examples

SUBDIRS += src

!qtlogger_no_tests {
    SUBDIRS += tests
    tests.depends = src
}

!qtlogger_no_examples {
    SUBDIRS += examples
    examples.depends = src
}

DISTFILES += \
    $$PWD/docs/qtlogger.conf.example \
    $$PWD/LICENSE \
    $$PWD/README.md

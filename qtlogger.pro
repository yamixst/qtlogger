# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

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
    $$PWD/doc/qtlogger.conf.example \
    $$PWD/LICENSE \
    $$PWD/README.md

#[[
# Copyright (C) 2026 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT
#]]

file(READ "${CMAKE_CURRENT_LIST_DIR}/../src/qtlogger/version.h" _qtlogger_version_header)

string(REGEX MATCH "#define[ \t]+QTLOGGER_VERSION[ \t]+([0-9]+\.[0-9]+\.[0-9]+)"
       _qtlogger_version_match
       "${_qtlogger_version_header}")

if(NOT _qtlogger_version_match)
    message(FATAL_ERROR "Unable to determine QTLOGGER_VERSION from src/qtlogger/version.h")
endif()

set(QTLOGGER_PROJECT_VERSION "${CMAKE_MATCH_1}")

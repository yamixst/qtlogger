// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QtCore/qglobal.h>

#if defined(QTLOGGER_STATIC)
#    define QTLOGGER_EXPORT
#elif defined(QTLOGGER_LIBRARY)
#    define QTLOGGER_EXPORT Q_DECL_EXPORT
#else
#    define QTLOGGER_EXPORT Q_DECL_IMPORT
#endif

#if !defined(QTLOGGER_DECL_SPEC)
#    define QTLOGGER_DECL_SPEC
#endif

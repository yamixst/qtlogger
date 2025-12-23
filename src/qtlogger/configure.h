// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QFlags>
#include <QSettings>
#include <QString>

#include "logger_global.h"
#include "sinks/rotatingfilesink.h"

namespace QtLogger {

class Logger;
class Pipeline;

QTLOGGER_EXPORT void configure(Pipeline *pipeline, const QString &path = {}, int maxFileSize = 0,
                               int maxFileCount = 0,
                               RotatingFileSink::Options options = RotatingFileSink::Option::None,
                               bool async = true);

QTLOGGER_EXPORT void configure(Pipeline *pipeline, const QSettings &settings,
                               const QString &group = QStringLiteral("logger"));

QTLOGGER_EXPORT void configureFromIniFile(Pipeline *pipeline, const QString &path,
                                          const QString &group = QStringLiteral("logger"));
} // namespace QtLogger

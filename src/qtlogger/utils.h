// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <qlogging.h>

#include "logger_global.h"

namespace QtLogger {

/** Set global filter rules
 *
 *  Format:  "[<category>|*].[debug|info|warning|critical]=true|false;..."
 *  Example: "app.*.debug=false;app.logger.debug=true"
 */

QTLOGGER_EXPORT void setFilterRules(const QString &rules);

/** Set global message pattern
 *
 * Following placeholders are supported:
 * %{appname} %{category} %{file} %{function} %{line} %{message} %{pid} %{threadid}
 * %{qthreadptr} %{type} %{time process} %{time boot} %{time [format]} %{backtrace [depth=N]
 * [separator="..."]}
 */

QTLOGGER_EXPORT QString setMessagePattern(const QString &messagePattern);

QTLOGGER_EXPORT QString restorePreviousMessagePattern();

} // namespace QtLogger

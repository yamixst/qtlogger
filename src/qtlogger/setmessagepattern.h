// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <qlogging.h>
#include "logger_global.h"

namespace QtLogger {

QTLOGGER_EXPORT QString setMessagePattern(const QString &messagePattern);

QTLOGGER_EXPORT QString restorePreviousMessagePattern();

} // namespace QtLogger

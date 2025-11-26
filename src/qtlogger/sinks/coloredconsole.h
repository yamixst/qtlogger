// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QString>
#include <qlogging.h>

#include "../logger_global.h"

namespace QtLogger {

enum class ColorMode {
    Auto, // Enable colors if output is a TTY
    Always, // Always enable colors
    Never // Never use colors
};

class QTLOGGER_EXPORT ColoredConsole
{
public:
    explicit ColoredConsole(ColorMode colorMode = ColorMode::Auto);
    virtual ~ColoredConsole();

    void setColorMode(ColorMode mode);
    ColorMode colorMode() const;
    bool colorsEnabled() const;

    static QString colorPrefix(QtMsgType type);
    static QString colorReset();
    static QString colorize(const QString &message, QtMsgType type);
    static bool isStdOutTty();
    static bool isStdErrTty();

protected:
    virtual bool isTty() const = 0;

    void updateColorsEnabled();

    ColorMode m_colorMode = ColorMode::Auto;
    bool m_colorsEnabled = false;
};

} // namespace QtLogger

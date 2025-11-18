// Copyright (C) 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include "coloredconsole.h"

#ifdef Q_OS_WIN
#    include <io.h>
#    include <stdio.h>
#    ifndef STDOUT_FILENO
#        define STDOUT_FILENO _fileno(stdout)
#    endif
#    ifndef STDERR_FILENO
#        define STDERR_FILENO _fileno(stderr)
#    endif
#    define isatty _isatty
#else
#    include <unistd.h>
#endif

namespace QtLogger {

QTLOGGER_DECL_SPEC
ColoredConsole::ColoredConsole(ColorMode colorMode) : m_colorMode(colorMode) { }

QTLOGGER_DECL_SPEC
ColoredConsole::~ColoredConsole() = default;

QTLOGGER_DECL_SPEC
void ColoredConsole::setColorMode(ColorMode mode)
{
    m_colorMode = mode;
    updateColorsEnabled();
}

QTLOGGER_DECL_SPEC
ColorMode ColoredConsole::colorMode() const
{
    return m_colorMode;
}

QTLOGGER_DECL_SPEC
bool ColoredConsole::colorsEnabled() const
{
    return m_colorsEnabled;
}

QTLOGGER_DECL_SPEC
QString ColoredConsole::colorPrefix(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("\033[90m"); // Gray
    case QtInfoMsg:
        return QStringLiteral("\033[32m"); // Green
    case QtWarningMsg:
        return QStringLiteral("\033[33m"); // Yellow
    case QtCriticalMsg:
        return QStringLiteral("\033[31m"); // Red
    case QtFatalMsg:
        return QStringLiteral("\033[1;91m"); // Bold bright red
    default:
        return QString();
    }
}

QTLOGGER_DECL_SPEC
QString ColoredConsole::colorReset()
{
    return QStringLiteral("\033[0m");
}

QTLOGGER_DECL_SPEC
QString ColoredConsole::colorize(const QString &message, QtMsgType type)
{
    const QString prefix = colorPrefix(type);
    if (prefix.isEmpty()) {
        return message;
    }
    return prefix + message + colorReset();
}

QTLOGGER_DECL_SPEC
bool ColoredConsole::isStdOutTty()
{
    static const bool isTty = isatty(STDOUT_FILENO) != 0;
    return isTty;
}

QTLOGGER_DECL_SPEC
bool ColoredConsole::isStdErrTty()
{
    static const bool isTty = isatty(STDERR_FILENO) != 0;
    return isTty;
}

QTLOGGER_DECL_SPEC
void ColoredConsole::updateColorsEnabled()
{
    switch (m_colorMode) {
    case ColorMode::Always:
        m_colorsEnabled = true;
        break;
    case ColorMode::Never:
        m_colorsEnabled = false;
        break;
    case ColorMode::Auto:
    default:
        m_colorsEnabled = isTty();
        break;
    }
}

} // namespace QtLogger

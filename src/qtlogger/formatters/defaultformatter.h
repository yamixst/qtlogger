// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "../handler.h"
#include "../logger_global.h"
#include "qtlogmessageformatter.h"

namespace QtLogger {

using DefaultFormatterPtr = QSharedPointer<class DefaultFormatter>;

class QTLOGGER_EXPORT DefaultFormatter : public Handler
{
public:
    static DefaultFormatterPtr instance();

    bool process(LogMessage &logMsg) override;

    HandlerPtr formatter() const;
    void setFormatter(HandlerPtr formatter);

private:
    explicit DefaultFormatter(const HandlerPtr &formatter);

    HandlerPtr m_formatter;
};

inline DefaultFormatterPtr DefaultFormatter::instance()
{
    static const auto s_instance = DefaultFormatterPtr(new DefaultFormatter(QtLogMessageFormatter::instance()));
    return s_instance;
}

inline DefaultFormatter::DefaultFormatter(const HandlerPtr &formatter)
    : m_formatter(formatter)
{
}

inline bool DefaultFormatter::process(LogMessage &logMsg)
{
    if (!logMsg.isFormatted() && m_formatter) {
        m_formatter->process(logMsg);
    }

    return true;
}

inline HandlerPtr DefaultFormatter::formatter() const
{
    return m_formatter;
}

inline void DefaultFormatter::setFormatter(HandlerPtr formatter)
{
    m_formatter = formatter;
}

} // namespace QtLogger

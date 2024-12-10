// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "../messagehandler.h"
#include "../logger_global.h"
#include "qtlogmessageformatter.h"

namespace QtLogger {

using DefaultFormatterPtr = QSharedPointer<class DefaultFormatter>;

class QTLOGGER_EXPORT DefaultFormatter : public MessageHandler
{
public:
    static DefaultFormatterPtr instance();

    bool process(LogMessage &logMsg) override;

    MessageHandlerPtr formatter() const;
    void setFormatter(MessageHandlerPtr formatter);

private:
    explicit DefaultFormatter(const MessageHandlerPtr &formatter);

    MessageHandlerPtr m_formatter;
};

inline DefaultFormatterPtr DefaultFormatter::instance()
{
    static const auto s_instance = DefaultFormatterPtr(new DefaultFormatter(QtLogMessageFormatter::instance()));
    return s_instance;
}

inline DefaultFormatter::DefaultFormatter(const MessageHandlerPtr &formatter)
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

inline MessageHandlerPtr DefaultFormatter::formatter() const
{
    return m_formatter;
}

inline void DefaultFormatter::setFormatter(MessageHandlerPtr formatter)
{
    m_formatter = formatter;
}

} // namespace QtLogger

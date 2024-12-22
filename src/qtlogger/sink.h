// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "messagehandler.h"
#include "logger_global.h"

class QMessageLogContext;

namespace QtLogger {

using SinkPtr = QSharedPointer<class Sink>;

class QTLOGGER_EXPORT Sink : public MessageHandler
{
public:
    virtual void send(const LogMessage &logMsg) = 0;

    virtual bool flush() { return true; }

    HandlerType type() const override { return HandlerType::Sink; }

    bool process(LogMessage &logMsg) override final
    {
        if (!m_preprocessor.isNull()) {
            auto _logMsg = logMsg;
            m_preprocessor->process(_logMsg);
            send(_logMsg);
            return true;
        }

        send(logMsg);

        return true;
    }

    MessageHandlerPtr preprocessor() const { return m_preprocessor; }

    inline void setPreprocessor(const MessageHandlerPtr &preprocessor)
    {
        if (preprocessor.data() == this)
            return;

        m_preprocessor = preprocessor;
    }

private:
    MessageHandlerPtr m_preprocessor;
};

} // namespace QtLogger

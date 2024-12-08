// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QSharedPointer>

#include "abstractmessageprocessor.h"
#include "logger_global.h"

class QMessageLogContext;

namespace QtLogger {

using AbstractMessageSinkPtr = QSharedPointer<class AbstractMessageSink>;

class QTLOGGER_EXPORT AbstractMessageSink : public AbstractMessageProcessor
{
public:
    virtual void send(const DebugMessage &dmesg) = 0;

    virtual bool flush() { return true; }

    Type processorType() const override { return AbstractMessageProcessor::Sink; }

    bool process(DebugMessage &dmesg) override final
    {
        if (!m_preprocessor.isNull()) {
            auto _dmesg = dmesg;
            m_preprocessor->process(_dmesg);
            send(_dmesg);
            return true;
        }

        send(dmesg);

        return true;
    }

    AbstractMessageProcessorPtr preprocessor() const { return m_preprocessor; }

    inline void setPreprocessor(const AbstractMessageProcessorPtr &preprocessor)
    {
        if (preprocessor.data() == this)
            return;

        m_preprocessor = preprocessor;
    }

private:
    AbstractMessageProcessorPtr m_preprocessor;
};

} // namespace QtLogger

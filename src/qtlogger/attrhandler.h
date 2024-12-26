#pragma once

#include <QSharedPointer>

#include "handler.h"

#include "logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT AttrHandler : public Handler
{
public:
    virtual QVariantHash attributes() const = 0;

    HandlerType type() const override { return HandlerType::AttrHandler; }

    bool process(LogMessage &logMsg) override
    {
        logMsg.attributes().insert(attributes());
        return true;
    }
};

using AttrHandlerPtr = QSharedPointer<AttrHandler>;

} // namespace QtLogger

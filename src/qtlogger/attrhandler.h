#pragma once

#include <QSharedPointer>

#include "handler.h"

#include "logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT AttrHandler : public Handler
{
public:
    virtual ~AttrHandler() = default;

    virtual QVariantHash attributes(const LogMessage &lmsg) = 0;

    HandlerType type() const override { return HandlerType::AttrHandler; }

    bool process(LogMessage &lmsg) override
    {
        lmsg.updateAttributes(attributes(lmsg));
        return true;
    }
};

using AttrHandlerPtr = QSharedPointer<AttrHandler>;

} // namespace QtLogger

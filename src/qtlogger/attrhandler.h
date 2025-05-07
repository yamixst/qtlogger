#pragma once

#include <QSharedPointer>

#include "handler.h"

#include "logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT AttrHandler : public Handler
{
public:
    virtual QVariantHash attributes(const LogMessage &lmsg) = 0;

    HandlerType type() const override { return HandlerType::AttrHandler; }

    bool process(LogMessage &lmsg) override
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        lmsg.attributes().insert(attributes(lmsg));
#else
        lmsg.attributes().unite(attributes(lmsg));
#endif
        return true;
    }
};

using AttrHandlerPtr = QSharedPointer<AttrHandler>;

} // namespace QtLogger

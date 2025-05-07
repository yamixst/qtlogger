#pragma once

#include <QSharedPointer>

#include "../attrhandler.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT FunctionAttrHandler : public AttrHandler
{
public:
    using Function = std::function<QVariantHash(const LogMessage &lmsg)>;

    FunctionAttrHandler(const Function &function) : m_function(function) { }

    QVariantHash attributes(const LogMessage &lmsg) override { return m_function(lmsg); }

private:
    Function m_function;
};

using FunctionAttrHandlerPtr = QSharedPointer<FunctionAttrHandler>;

} // namespace QtLogger

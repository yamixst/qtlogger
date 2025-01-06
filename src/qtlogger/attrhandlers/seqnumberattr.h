#pragma once

#include <QSharedPointer>

#include "../attrhandler.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT SeqNumberAttr : public AttrHandler
{
public:
    QVariantHash attributes() override;

private:
    int m_count = 0;
};

using SeqNumberAttrPtr = QSharedPointer<SeqNumberAttr>;

} // namespace QtLogger

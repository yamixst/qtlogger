#pragma once

#include <QSharedPointer>

#include "../attrhandler.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT AppInfoAttrs : public AttrHandler
{
public:
    QVariantHash attributes() const override;
};

using AppInfoAttrsPtr = QSharedPointer<AppInfoAttrs>;

} // namespace QtLogger

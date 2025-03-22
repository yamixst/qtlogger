#pragma once

#include <QSharedPointer>

#include "../attrhandler.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT AppInfoAttrs : public AttrHandler
{
public:
    AppInfoAttrs();

    QVariantHash attributes() override;

private:
    QVariantHash m_attrs;
};

using AppInfoAttrsPtr = QSharedPointer<AppInfoAttrs>;

} // namespace QtLogger

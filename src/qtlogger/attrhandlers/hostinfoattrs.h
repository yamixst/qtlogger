#pragma once

#ifdef QTLOGGER_NETWORK

#include <QSharedPointer>

#include "../attrhandler.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT HostInfoAttrs : public AttrHandler
{
public:
    QVariantHash attributes() override;
};

using HostInfoAttrsPtr = QSharedPointer<HostInfoAttrs>;

} // namespace QtLogger

#endif // QTLOGGER_NETWORK

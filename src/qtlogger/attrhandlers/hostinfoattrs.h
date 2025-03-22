#pragma once

#ifdef QTLOGGER_NETWORK

#include <QSharedPointer>

#include "../attrhandler.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT HostInfoAttrs : public AttrHandler
{
public:
    HostInfoAttrs();

    QVariantHash attributes() override;

private:
    QVariantHash m_attrs;
};

using HostInfoAttrsPtr = QSharedPointer<HostInfoAttrs>;

} // namespace QtLogger

#endif // QTLOGGER_NETWORK

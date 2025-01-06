#ifdef QTLOGGER_NETWORK

#include "hostinfoattrs.h"

#include <QHostInfo>

namespace QtLogger {

QTLOGGER_DECL_SPEC
QVariantHash HostInfoAttrs::attributes()
{
    return QVariantHash {
        { QStringLiteral("host_name"), QHostInfo::localHostName() },
    };
}

} // namespace QtLogger

#endif // QTLOGGER_NETWORK

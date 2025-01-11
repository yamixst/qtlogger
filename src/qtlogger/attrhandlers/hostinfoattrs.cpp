#ifdef QTLOGGER_NETWORK

#include "hostinfoattrs.h"

#include <QHostInfo>

namespace QtLogger {

QVariantHash HostInfoAttrs::attributes() const
{
    return QVariantHash {
        { QStringLiteral("host_name"), QHostInfo::localHostName() },
    };
}

} // namespace QtLogger

#endif // QTLOGGER_NETWORK

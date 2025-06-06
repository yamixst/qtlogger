#ifdef QTLOGGER_NETWORK

#    include "hostinfoattrs.h"

#    include <QHostInfo>

namespace QtLogger {

QTLOGGER_DECL_SPEC
HostInfoAttrs::HostInfoAttrs()
{
    m_attrs = QVariantHash {
        { QStringLiteral("host_name"), QHostInfo::localHostName() },
    };
}

QTLOGGER_DECL_SPEC
QVariantHash HostInfoAttrs::attributes()
{
    return m_attrs;
}

} // namespace QtLogger

#endif // QTLOGGER_NETWORK

#include "appinfoattrs.h"

#include <QCoreApplication>

namespace QtLogger {

QTLOGGER_DECL_SPEC
AppInfoAttrs::AppInfoAttrs()
{
    m_attrs = QVariantHash {
        { QStringLiteral("appname"), QCoreApplication::applicationName() },
        { QStringLiteral("appversion"), QCoreApplication::applicationVersion() },
        { QStringLiteral("appdir"), QCoreApplication::applicationDirPath() },
        { QStringLiteral("apppath"), QCoreApplication::applicationFilePath() },
        { QStringLiteral("pid"), QCoreApplication::applicationPid() },
    };
}

QTLOGGER_DECL_SPEC
QVariantHash AppInfoAttrs::attributes(const LogMessage &lmsg)
{
    Q_UNUSED(lmsg)
    return m_attrs;
}

} // namespace QtLogger

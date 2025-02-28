#include "appinfoattrs.h"

#include <QCoreApplication>

namespace QtLogger {

QTLOGGER_DECL_SPEC
AppInfoAttrs::AppInfoAttrs()
{
    m_attrs = QVariantHash {
        { QStringLiteral("app_name"), QCoreApplication::applicationName() },
        { QStringLiteral("app_version"), QCoreApplication::applicationVersion() },
        { QStringLiteral("app_dir"), QCoreApplication::applicationDirPath() },
        { QStringLiteral("app_path"), QCoreApplication::applicationFilePath() },
        { QStringLiteral("pid"), QCoreApplication::applicationPid() },
    };
}

QVariantHash AppInfoAttrs::attributes(const LogMessage &lmsg)
{
    Q_UNUSED(lmsg)
    return m_attrs;
}

} // namespace QtLogger

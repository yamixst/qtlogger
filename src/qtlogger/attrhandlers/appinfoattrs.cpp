#include "appinfoattrs.h"

#include <QCoreApplication>

namespace QtLogger {

QVariantHash AppInfoAttrs::attributes() const
{
    return QVariantHash {
        { QStringLiteral("app_name"), QCoreApplication::applicationName() },
        { QStringLiteral("app_version"), QCoreApplication::applicationVersion() },
        { QStringLiteral("app_dir"), QCoreApplication::applicationDirPath() },
        { QStringLiteral("app_path"), QCoreApplication::applicationFilePath() },
        { QStringLiteral("pid"), QCoreApplication::applicationPid() },
    };
}

} // namespace QtLogger

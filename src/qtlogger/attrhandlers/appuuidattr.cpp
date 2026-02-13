// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Xstream

#include "appuuidattr.h"

#include <QCoreApplication>
#include <QSettings>
#include <QUuid>

namespace QtLogger {

QTLOGGER_DECL_SPEC
AppUuidAttr::AppUuidAttr(const QString &name) : m_name(name)
{
    QSettings settings(QSettings::UserScope, QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());

    auto uuid = settings.value(QStringLiteral("app_uuid")).toString();

    if (uuid.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
#else
        uuid = QUuid::createUuid().toString();
        uuid.remove(QLatin1Char('{')).remove(QLatin1Char('}'));
#endif
        settings.setValue(QStringLiteral("app_uuid"), uuid);
    }

    m_uuid = uuid;
}

QTLOGGER_DECL_SPEC
QVariantHash AppUuidAttr::attributes(const LogMessage &lmsg)
{
    Q_UNUSED(lmsg)
    return { { m_name, m_uuid } };
}

} // namespace QtLogger
// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "jsonformatter.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>

#ifdef QTLOGGER_NETWORK
#    include <QHostInfo>
#endif

namespace QtLogger {

QTLOGGER_DECL_SPEC
QString JsonFormatter::format(const LogMessage &logMsg) const
{
    static QStringList types { QStringLiteral("debug"),    QStringLiteral("warning"),
                               QStringLiteral("critical"), QStringLiteral("fatal"),
                               QStringLiteral("info"),     QStringLiteral("system") };

    QJsonObject obj;

    obj[QStringLiteral("message")] = logMsg.message();
    obj[QStringLiteral("type")] = types.value(logMsg.type(), QStringLiteral("debug"));
    obj[QStringLiteral("line")] = logMsg.line();
    obj[QStringLiteral("file")] = QString::fromUtf8(logMsg.file());
    obj[QStringLiteral("function")] = QString::fromUtf8(logMsg.function());
    obj[QStringLiteral("category")] = QString::fromUtf8(logMsg.category());
    obj[QStringLiteral("time")] = logMsg.time().toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zZ"));
    obj[QStringLiteral("thread")] = logMsg.threadId();

    for (const auto &key : logMsg.attributes().keys()) {
        obj[key] = QJsonValue::fromVariant(logMsg.attribute(key));
    }

    if (qApp) {
        obj[QStringLiteral("pid")] = qApp->applicationPid();
        obj[QStringLiteral("app_path")] = qApp->applicationFilePath();
        obj[QStringLiteral("app_name")] = qApp->applicationName();
        obj[QStringLiteral("app_version")] = qApp->applicationVersion();
#ifdef QTLOGGER_NETWORK
        obj[QStringLiteral("host_name")] = QHostInfo::localHostName();
#endif
    }

    return QString::fromUtf8(QJsonDocument(obj).toJson());
}

} // namespace QtLogger

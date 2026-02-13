// SPDX-FileCopyrightText: 2026 Alexey Rochev
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QSharedPointer>

#include "../attrhandler.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT AppUuidAttr : public AttrHandler
{
public:
    explicit AppUuidAttr(const QString &name = QStringLiteral("app_uuid"));
    QVariantHash attributes(const LogMessage &lmsg) override;

private:
    QString m_name;
    QString m_uuid;
};

using AppUuidAttrPtr = QSharedPointer<AppUuidAttr>;

} // namespace QtLogger
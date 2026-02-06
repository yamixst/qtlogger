// Copyright (C) 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QSharedPointer>

#include "../attrhandler.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT SysInfoAttrs : public AttrHandler
{
public:
    SysInfoAttrs();

    QVariantHash attributes(const LogMessage &lmsg) override;

private:
    QVariantHash m_attrs;
};

using SysInfoAttrsPtr = QSharedPointer<SysInfoAttrs>;

} // namespace QtLogger
// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QHash>

#include "../formatter.h"
#include "../logger_global.h"

namespace QtLogger {

using PrettyFormatterPtr = QSharedPointer<class PrettyFormatter>;

class QTLOGGER_EXPORT PrettyFormatter : public Formatter
{
public:
    static PrettyFormatterPtr instance()
    {
        static const auto s_instance = PrettyFormatterPtr::create(0, false);
        return s_instance;
    }

    explicit PrettyFormatter(bool colorize = false, int maxCategoryWidth = 15);

    QString format(const LogMessage &lmsg) override;

private:
    bool m_colorize = false;
    int m_maxCategoryWidth = 15;

    QHash<int, int> m_threads;
    int m_threadsIndex = 0;
    int m_categoryWidth = 0;
};

} // namespace QtLogger

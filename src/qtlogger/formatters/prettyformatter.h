// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QMap>
#include <QStringList>

#include "../formatter.h"
#include "../logger_global.h"

namespace QtLogger {

using PrettyFormatterPtr = QSharedPointer<class PrettyFormatter>;

class QTLOGGER_EXPORT PrettyFormatter : public Formatter
{
public:
    static PrettyFormatterPtr instance()
    {
        static const auto s_instance = PrettyFormatterPtr::create(true, 0, false);
        return s_instance;
    }

    explicit PrettyFormatter(bool showThread = true, int maxCategoryWidth = 15,
                             bool colorize = false);

    QString format(const LogMessage &lmsg) override;

private:
    bool m_showThreadId = true;
    bool m_colorize = false;
    QMap<int, int> m_threads;
    int m_threadsIndex = 0;
    int m_maxCategoryWidth = 15;
    int m_categoryWidth = 0;
};

} // namespace QtLogger

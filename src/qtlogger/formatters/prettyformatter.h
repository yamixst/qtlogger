// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

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
        static const auto s_instance = PrettyFormatterPtr::create(true, 0);
        return s_instance;
    }

    explicit PrettyFormatter(bool showThread = true, int maxCategoryWidth = 15);

    QString format(const LogMessage &logMsg) const override;

    inline bool showThreadId() const { return m_showThreadId; }
    inline void setShowThreadId(bool newShowThreadId) { m_showThreadId = newShowThreadId; }

    inline int maxCategoryWidth() const { return m_maxCategoryWidth; }
    inline void setMaxCategoryWidth(int newMaxCategoryWidth)
    {
        m_maxCategoryWidth = newMaxCategoryWidth;
    }

private:
    bool m_showThreadId = true;
    mutable QMap<int, int> m_threads;
    mutable int m_threadsIndex = 0;

    int m_maxCategoryWidth = 15;
    mutable int m_categoryWidth = 0;
};

} // namespace QtLogger

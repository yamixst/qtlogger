// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QMap>
#include <QStringList>

#include "../abstractmessageprocessor.h"
#include "../logger_global.h"

namespace QtLogger {

using PrettyFormatterPtr = QSharedPointer<class PrettyFormatter>;

class QTLOGGER_EXPORT PrettyFormatter : public AbstractMessageProcessor
{
public:
    static PrettyFormatterPtr instance()
    {
        static const auto s_instance = PrettyFormatterPtr::create(true, 0);
        return s_instance;
    }

    explicit PrettyFormatter(bool showThread = true, int maxCategoryWidth = 15);

    bool process(DebugMessage &dmesg) override final;

    inline bool showThread() const { return m_showThread; }
    inline void setShowThread(bool newShowThread) { m_showThread = newShowThread; }

    inline int maxCategoryWidth() const { return m_maxCategoryWidth; }
    inline void setMaxCategoryWidth(int newMaxCategoryWidth)
    {
        m_maxCategoryWidth = newMaxCategoryWidth;
    }

private:
    bool m_showThread = true;
    QMap<int, int> m_threads;
    int m_threadsIndex = 0;

    int m_maxCategoryWidth = 15;
    int m_categoryWidth = 0;
};

} // namespace QtLogger

#pragma once

#include <QSharedPointer>

#include "../filter.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT LevelFilter : public Filter
{
public:
    explicit LevelFilter(QtMsgType minLevel = QtDebugMsg) : m_minLevel(minLevel) { }

    bool filter(const LogMessage &lmsg) override {
        return priority(lmsg.type()) >= priority(m_minLevel);
    }

private:
    static int priority(QtMsgType type) {
        switch (type) {
            case QtDebugMsg:    return 0;
            case QtInfoMsg:     return 1;
            case QtWarningMsg:  return 2;
            case QtCriticalMsg: return 3;
            case QtFatalMsg:    return 4;
        }
        return -1;
    }

    QtMsgType m_minLevel;
};

using LevelFilterPtr = QSharedPointer<LevelFilter>;

} // namespace QtLogger

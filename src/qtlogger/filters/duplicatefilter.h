#pragma once

#include "../filter.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT DuplicateFilter : public Filter
{
public:
    bool filter(const LogMessage &lmsg) override;

private:
    QString m_lastMessage;
};

using DuplicateFilterPtr = QSharedPointer<DuplicateFilter>;

} // namespace QtLogger

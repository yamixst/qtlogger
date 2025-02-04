#include "duplicatefilter.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
bool DuplicateFilter::filter(const LogMessage &logMsg)
{
    if (logMsg.message() == m_lastMessage) {
        return false;
    }

    m_lastMessage = logMsg.message();
    return true;
}

} // namespace QtLogger

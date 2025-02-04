#include "duplicatefilter.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
bool DuplicateFilter::filter(const LogMessage &lmsg)
{
    if (lmsg.message() == m_lastMessage) {
        return false;
    }

    m_lastMessage = lmsg.message();
    return true;
}

} // namespace QtLogger

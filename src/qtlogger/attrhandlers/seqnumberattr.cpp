#include "seqnumberattr.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
QVariantHash SeqNumberAttr::attributes(const LogMessage &lmsg)
{
    Q_UNUSED(lmsg)
    return { { QStringLiteral("seq_number"), m_count++ } };
}

} // namespace QtLogger

#include "seqnumberattr.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
SeqNumberAttr::SeqNumberAttr(const QString &name) : m_name(name) { }

QVariantHash SeqNumberAttr::attributes(const LogMessage &lmsg)
{
    Q_UNUSED(lmsg)
    return { { m_name, m_count++ } };
}

} // namespace QtLogger

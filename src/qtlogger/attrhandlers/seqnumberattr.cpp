#include "seqnumberattr.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
QVariantHash SeqNumberAttr::attributes() const
{
    return { { "seq_number", m_count++ } };
}

} // namespace QtLogger

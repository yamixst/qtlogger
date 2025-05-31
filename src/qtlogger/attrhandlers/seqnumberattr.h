#pragma once

#include <QSharedPointer>

#include "../attrhandler.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT SeqNumberAttr : public AttrHandler
{
public:
    explicit SeqNumberAttr(const QString &name = QStringLiteral("seq_number"));
    QVariantHash attributes(const LogMessage &lmsg) override;

private:
    QString m_name;
    int m_count = 0;
};

using SeqNumberAttrPtr = QSharedPointer<SeqNumberAttr>;

} // namespace QtLogger

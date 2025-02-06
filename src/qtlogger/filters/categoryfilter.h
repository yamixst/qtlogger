#pragma once

#include <QSharedPointer>

#include "../filter.h"
#include "../logger_global.h"

namespace QtLogger {

class QTLOGGER_EXPORT CategoryFilter : public Filter
{
public:
    CategoryFilter(const QString &rules);

    bool filter(const LogMessage &lmsg) override;

private:
    struct Rule;
    void parseRules(const QString &rules);
    QList<QSharedPointer<Rule>> m_rules;
};

using CategoryFilterPtr = QSharedPointer<CategoryFilter>;

} // namespace QtLogger

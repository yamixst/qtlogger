#include "categoryfilter.h"

#include <QRegularExpression>
#include <qlogging.h>

namespace QtLogger {

struct CategoryFilter::Rule
{
    QRegularExpression category;
    QtMsgType type;
    bool typeMatch;
    bool enabled;

    bool matches(const QString &category, QtMsgType messageType) const;
};

QTLOGGER_DECL_SPEC
CategoryFilter::CategoryFilter(const QString &a_rules)
{
    auto rules = a_rules;
    rules.replace(";", "\n");
    parseRules(rules);
}

QTLOGGER_DECL_SPEC
void CategoryFilter::parseRules(const QString &rules)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    const auto lines = rules.split('\n', Qt::SkipEmptyParts);
#else
    const auto lines = rules.split('\n', QString::SkipEmptyParts);
#endif
    for (const auto &line : lines) {
        const auto ruleRegex = QRegularExpression(
                R"(^\s*(\S+?)(?:\.(debug|info|warning|critical))?\s*=\s*(true|false)\s*$)");

        const auto match = ruleRegex.match(line);

        if (!match.hasMatch())
            continue;

        auto rule = QSharedPointer<Rule>::create();

        auto category = match.captured(1);
        category = QRegularExpression::escape(category);
        category.replace("\\*", ".*");

        rule->category = QRegularExpression("^" + category + "$");
        rule->type = stringToQtMsgType(match.captured(2));
        rule->typeMatch = !match.captured(2).isEmpty();
        rule->enabled = match.captured(3) == "true";

        m_rules.append(rule);
    }
}

QTLOGGER_DECL_SPEC
bool CategoryFilter::Rule::matches(const QString &category, QtMsgType messageType) const
{
    return this->category.match(category).hasMatch() && (!typeMatch || type == messageType);
}

QTLOGGER_DECL_SPEC
bool CategoryFilter::filter(const LogMessage &lmsg)
{
    bool enabled = true;
    for (const auto &rule : m_rules) {
        if (rule->matches(lmsg.category(), lmsg.type())) {
            enabled = rule->enabled;
        }
    }
    return enabled;
}

} // namespace QtLogger

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "patternformatter.h"

#include <QSharedPointer>

namespace QtLogger {

namespace {

class Token
{
public:
    virtual ~Token() = default;
    virtual size_t estimatedLength() const = 0;
    virtual bool checkCondition(const LogMessage &) const { return true; }
    virtual void appendToString(const LogMessage &lmsg, QString &dest) const = 0;
};

class ConditionToken : public Token
{
public:
    bool checkCondition(const LogMessage &lmsg) const override
    {
        if (!m_hasCondition) {
            return true;
        }
        return lmsg.type() == m_condition;
    }

    QtMsgType condition() const { return m_condition; }

    void setCondition(QtMsgType condition)
    {
        m_condition = condition;
        m_hasCondition = true;
    }

private:
    QtMsgType m_condition = QtDebugMsg;
    bool m_hasCondition = false;
};

class LiteralToken : public ConditionToken
{
public:
    explicit LiteralToken(const QString &text) : m_text(text) { }

    void appendToString(const LogMessage &, QString &dest) const override
    {
        dest.append(m_text);
    }

    size_t estimatedLength() const override { return m_text.size(); }

private:
    QString m_text;
};

class MessageToken : public ConditionToken
{
public:
    MessageToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(lmsg.message());
    }

    size_t estimatedLength() const override
    {
        return 50; // Estimated average message length
    }
};

class TypeToken : public ConditionToken
{
public:
    TypeToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(qtMsgTypeToString(lmsg.type()));
    }

    size_t estimatedLength() const override
    {
        return 8; // Maximum length of "critical"
    }
};

class LineToken : public ConditionToken
{
public:
    LineToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(QString::number(lmsg.line()));
    }

    size_t estimatedLength() const override
    {
        return 5; // Maximum length of "99999"
    }
};

class FileToken : public ConditionToken
{
public:
    FileToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(lmsg.file());
    }

    size_t estimatedLength() const override
    {
        return 20; // Maximum length of "path/to/file.cpp"
    }
};

class FunctionToken : public ConditionToken
{
public:
    FunctionToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(lmsg.function());
    }

    size_t estimatedLength() const override
    {
        return 20; // Maximum length of "functionName"
    }
};

class CategoryToken : public ConditionToken
{
public:
    CategoryToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(lmsg.category());
    }

    size_t estimatedLength() const override
    {
        return 20; // Maximum length of "categoryName"
    }
};

class TimeToken : public ConditionToken
{
public:
    TimeToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(lmsg.time().toString(Qt::ISODate));
    }

    size_t estimatedLength() const override
    {
        return 20; // Maximum length of "2023-01-01T00:00:00"
    }
};

class ThreadIdToken : public ConditionToken
{
public:
    ThreadIdToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(QString::number(lmsg.threadId()));
    }

    size_t estimatedLength() const override
    {
        return 10; // Maximum length of "9999999999"
    }
};

} // namespace

class PatternFormatter::PatternFormatterPrivate
{
public:
    explicit PatternFormatterPrivate(const QString &pattern) : m_pattern(pattern)
    {
        parsePattern();
    }

    void parsePattern()
    {
        m_tokens.clear();

        int pos = 0;
        QString literalText;
        QtMsgType currentCondition = QtDebugMsg;
        bool hasCondition = false;

        while (pos < m_pattern.length()) {
            if (pos < m_pattern.length() - 1 && m_pattern[pos] == '%') {
                if (m_pattern[pos + 1] == '{') {
                    if (!literalText.isEmpty()) {
                        auto token = new LiteralToken(literalText);
                        if (hasCondition) {
                            token->setCondition(currentCondition);
                        }
                        m_tokens.append(QSharedPointer<Token>(token));
                        literalText.clear();
                    }

                    int closingPos = m_pattern.indexOf('}', pos + 2);
                    if (closingPos == -1) {
                        // No closing brace, treat as literal
                        literalText.append('%');
                        pos++;
                        continue;
                    }

                    QString placeholder = m_pattern.mid(pos + 2, closingPos - pos - 2);

                    ConditionToken *token = nullptr;

                    if (placeholder == "type") {
                        token = new TypeToken();
                    } else if (placeholder == "line") {
                        token = new LineToken();
                    } else if (placeholder == "file") {
                        token = new FileToken();
                    } else if (placeholder == "function") {
                        token = new FunctionToken();
                    } else if (placeholder == "category") {
                        token = new CategoryToken();
                    } else if (placeholder == "time") {
                        token = new TimeToken();
                    } else if (placeholder == "threadid") {
                        token = new ThreadIdToken();
                    } else if (placeholder == "message") {
                        token = new MessageToken();
                    } else if (placeholder.startsWith("if-")) {
                        // Handle conditional: %{if-debug}, %{if-warning}, etc.
                        QString conditionType = placeholder.mid(3); // Remove "if-"
                        currentCondition = stringToQtMsgType(conditionType, QtDebugMsg);
                        hasCondition = true;
                        pos = closingPos + 1;
                        continue;
                    } else if (placeholder == "endif") {
                        hasCondition = false;
                        pos = closingPos + 1;
                        continue;
                    } else {
                        // Unknown placeholder, treat as literal
                        literalText.append(m_pattern.mid(pos, closingPos - pos + 1));
                        pos = closingPos + 1;
                        continue;
                    }

                    if (token) {
                        if (hasCondition) {
                            token->setCondition(currentCondition);
                        }
                        m_tokens.append(QSharedPointer<Token>(token));
                    }

                    pos = closingPos + 1;
                } else if (m_pattern[pos + 1] == '%') {
                    // Escaped %, add single %
                    literalText.append('%');
                    pos += 2;
                } else {
                    // Just a regular %
                    literalText.append('%');
                    pos++;
                }
            } else {
                literalText.append(m_pattern[pos]);
                pos++;
            }
        }

        if (!literalText.isEmpty()) {
            auto token = new LiteralToken(literalText);
            if (hasCondition) {
                token->setCondition(currentCondition);
            }
            m_tokens.append(QSharedPointer<Token>(token));
        }
    }

    QString format(const LogMessage &lmsg)
    {
        if (m_tokens.isEmpty()) {
            return lmsg.message();
        }

        size_t estimatedLength = 0;
        for (const auto &token : m_tokens) {
            if (token->checkCondition(lmsg)) {
                estimatedLength += token->estimatedLength();
            }
        }

        QString result;
        result.reserve(estimatedLength);

        for (const auto &token : m_tokens) {
            if (token->checkCondition(lmsg)) {
                token->appendToString(lmsg, result);
            }
        }

        return result;
    }

    QString m_pattern;
    QList<QSharedPointer<Token>> m_tokens;
};

QTLOGGER_DECL_SPEC
PatternFormatter::PatternFormatter(const QString &pattern) : d(new PatternFormatterPrivate(pattern))
{
}

QTLOGGER_DECL_SPEC
PatternFormatter::~PatternFormatter() = default;

QTLOGGER_DECL_SPEC
QString PatternFormatter::format(const LogMessage &lmsg)
{
    return d->format(lmsg);
}

} // namespace QtLogger

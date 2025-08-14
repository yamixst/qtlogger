// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "patternformatter.h"

#include <QSharedPointer>

namespace QtLogger {

namespace {

// Static variables for process start time
static const auto g_processStartTime = std::chrono::steady_clock::now();

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

    void appendToString(const LogMessage &, QString &dest) const override { dest.append(m_text); }

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

class ShortFileToken : public ConditionToken
{
public:
    ShortFileToken(const QString &baseDir = QString()) : m_baseDir(baseDir) { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        QString file = lmsg.file();
        if (m_baseDir.isEmpty()) {
            // No basedir specified - return only filename without directory
            int lastSlash = file.lastIndexOf(QLatin1Char('/'));
            if (lastSlash == -1) {
                lastSlash = file.lastIndexOf(QLatin1Char('\\'));
            }
            if (lastSlash != -1) {
                dest.append(file.mid(lastSlash + 1));
            } else {
                dest.append(file);
            }
        } else {
            // Strip basedir prefix if present
            if (file.startsWith(m_baseDir)) {
                QString result = file.mid(m_baseDir.length());
                // Remove leading slash if present
                if (result.startsWith(QLatin1Char('/')) || result.startsWith(QLatin1Char('\\'))) {
                    result = result.mid(1);
                }
                dest.append(result);
            } else {
                dest.append(file);
            }
        }
    }

    size_t estimatedLength() const override
    {
        return 20;
    }

private:
    QString m_baseDir;
};

class FunctionToken : public ConditionToken
{
public:
    FunctionToken(bool cleanup = true) : m_cleanup(cleanup) { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        if (m_cleanup) {
            dest.append(QString::fromLatin1(cleanup(lmsg.function())));
        } else {
            dest.append(QString::fromLatin1(lmsg.function()));
        }
    }

    size_t estimatedLength() const override
    {
        return m_cleanup ? 20 : 40;
    }

private:
    bool m_cleanup;

    static QByteArray cleanup(QByteArray func)
    {
        if (func.isEmpty())
            return func;

        // Helper lambda: find balanced bracket in reverse
        auto findBalancedReverse = [&func](char open, char close, int startPos) -> int {
            int count = 1;
            int pos = startPos - 1;
            while (pos >= 0 && count > 0) {
                char c = func.at(pos);
                if (c == close)
                    ++count;
                else if (c == open)
                    --count;
                --pos;
            }
            return (count == 0) ? pos + 1 : -1;
        };

        // Remove compiler metadata like [with T = int]
        if (func.endsWith(']') && !func.startsWith('+') && !func.startsWith('-')) {
            int openBracket = findBalancedReverse('[', ']', func.size() - 1);
            if (openBracket != -1)
                func.truncate(openBracket);
        }
        while (func.endsWith(' '))
            func.chop(1);

        // Normalize operator spacing
        func.replace("operator ", "operator");

        // Try to handle function pointer return types: returntype (*name(args))(return_args)
        bool handledFunctionPointer = false;
        int parenOpenIdx = func.indexOf(")(");
        if (parenOpenIdx != -1) {
            int ptrParen = func.indexOf("(*");
            if (ptrParen != -1 && ptrParen < parenOpenIdx) {
                int nameStart = ptrParen + 2;
                int parenDepth = 0;
                int argsParen = -1;
                for (int i = nameStart; i < parenOpenIdx; ++i) {
                    char c = func.at(i);
                    if (c == '(') {
                        if (parenDepth == 0)
                            argsParen = i;
                        ++parenDepth;
                    } else if (c == ')') {
                        --parenDepth;
                    }
                }
                if (argsParen != -1 && argsParen > nameStart) {
                    func = func.mid(nameStart, argsParen - nameStart);
                    handledFunctionPointer = true;
                }
            }
        }

        if (!handledFunctionPointer) {
            // Remove function arguments
            int end = func.lastIndexOf(')');
            if (end != -1) {
                int openParen = findBalancedReverse('(', ')', end);
                if (openParen != -1) {
                    bool isOperatorCall =
                            (openParen >= 8 && func.mid(openParen - 8, 8) == "operator");
                    if (!isOperatorCall)
                        func.truncate(openParen);
                }
            }

            // Remove trailing qualifiers
            static const char *const qualifiers[] = { " const", " volatile", " noexcept",
                                                      " override", " final" };
            bool found;
            do {
                found = false;
                for (const char *qual : qualifiers) {
                    if (func.endsWith(qual)) {
                        func.chop(static_cast<int>(qstrlen(qual)));
                        found = true;
                        break;
                    }
                }
            } while (found);

            // Extract function name (remove return type)
            int operatorPos = func.lastIndexOf("operator");
            if (operatorPos != -1) {
                int scanPos = operatorPos - 1;
                while (scanPos >= 0 && func.at(scanPos) == ' ')
                    --scanPos;

                bool extracted = false;
                while (scanPos >= 0) {
                    if (scanPos >= 1 && func.at(scanPos) == ':' && func.at(scanPos - 1) == ':') {
                        scanPos -= 2;
                        while (scanPos >= 0 && func.at(scanPos) == ' ')
                            --scanPos;
                        if (scanPos >= 0 && func.at(scanPos) == ')') {
                            int op = findBalancedReverse('(', ')', scanPos + 1);
                            if (op != -1) {
                                scanPos = op - 1;
                                continue;
                            }
                        }
                        if (scanPos >= 0 && func.at(scanPos) == '>') {
                            int oa = findBalancedReverse('<', '>', scanPos + 1);
                            if (oa != -1) {
                                scanPos = oa - 1;
                                continue;
                            }
                        }
                        while (scanPos >= 0
                               && (QChar(func.at(scanPos)).isLetterOrNumber()
                                   || func.at(scanPos) == '_'))
                            --scanPos;
                    } else if (func.at(scanPos) == ' ') {
                        func = func.mid(scanPos + 1);
                        extracted = true;
                        break;
                    } else {
                        break;
                    }
                }
                if (!extracted) {
                    int firstSpace = func.indexOf(' ');
                    if (firstSpace != -1 && firstSpace < operatorPos)
                        func = func.mid(firstSpace + 1);
                }
            } else {
                int pos = func.size() - 1;
                int parenCount = 0, angleCount = 0;
                while (pos >= 0) {
                    char c = func.at(pos);
                    if (c == ')') {
                        ++parenCount;
                        --pos;
                        continue;
                    }
                    if (c == '(' && parenCount > 0) {
                        --parenCount;
                        --pos;
                        continue;
                    }
                    if (c == '>') {
                        ++angleCount;
                        --pos;
                        continue;
                    }
                    if (c == '<' && angleCount > 0) {
                        --angleCount;
                        --pos;
                        continue;
                    }
                    if (parenCount > 0 || angleCount > 0) {
                        --pos;
                        continue;
                    }
                    if (c == ' ') {
                        func = func.mid(pos + 1);
                        break;
                    }
                    --pos;
                }
            }
            while (func.startsWith('*') || func.startsWith('&') || func.startsWith(' '))
                func = func.mid(1);
        }

        // Remove empty parentheses before :: (e.g., method():: -> method::)
        int pos = 0;
        while ((pos = func.indexOf("()::", pos)) != -1) {
            if (pos >= 8 && func.mid(pos - 8, 8) == "operator") {
                pos += 4;
                continue;
            }
            int angleDepth = 0;
            bool insideTemplate = false;
            for (int i = pos - 1; i >= 0; --i) {
                if (func.at(i) == '>')
                    ++angleDepth;
                else if (func.at(i) == '<') {
                    if (angleDepth == 0) {
                        insideTemplate = true;
                        break;
                    }
                    --angleDepth;
                }
            }
            if (insideTemplate) {
                pos += 4;
                continue;
            }
            func.remove(pos, 2);
        }

        // Remove template parameters
        while (true) {
            int closeAngle = func.lastIndexOf('>');
            if (closeAngle == -1)
                break;
            int opCheck = func.lastIndexOf("operator", closeAngle);
            if (opCheck != -1) {
                bool isOperatorSymbol = true;
                for (int i = opCheck + 8; i <= closeAngle; ++i) {
                    char ch = func.at(i);
                    if (ch != '<' && ch != '>' && !QByteArray("=!+-*/%^&|~").contains(ch)) {
                        isOperatorSymbol = false;
                        break;
                    }
                }
                if (isOperatorSymbol)
                    break;
            }
            int openAngle = findBalancedReverse('<', '>', closeAngle);
            if (openAngle == -1)
                break;
            if (openAngle >= 8 && func.mid(openAngle - 8, 8) == "operator")
                break;
            if (func.mid(openAngle + 1, closeAngle - openAngle - 1).startsWith("lambda"))
                break;
            func.remove(openAngle, closeAngle - openAngle + 1);
        }

        return func;
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
    explicit TimeToken(const QString &format = QString()) : m_format(format) { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        if (m_format == QLatin1String("process")) {
            // Time since process started in seconds
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    lmsg.steadyTime() - g_processStartTime);
            double seconds = duration.count() / 1000.0;
            dest.append(QString::number(seconds, 'f', 3));
        } else if (m_format == QLatin1String("boot")) {
            // Time since system boot in seconds using steady_clock epoch
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    lmsg.steadyTime().time_since_epoch());
            double seconds = duration.count() / 1000.0;
            dest.append(QString::number(seconds, 'f', 3));
        } else if (m_format.isEmpty()) {
            dest.append(lmsg.time().toString(Qt::ISODate));
        } else {
            dest.append(lmsg.time().toString(m_format));
        }
    }

    size_t estimatedLength() const override
    {
        if (m_format == QLatin1String("process") || m_format == QLatin1String("boot")) {
            return 15; // Enough for "123456789.123"
        }
        return m_format.isEmpty() ? 20 : m_format.length() * 2; // Estimated length based on format
    }

private:
    QString m_format;
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

class QThreadPtrToken : public ConditionToken
{
public:
    QThreadPtrToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(QStringLiteral("0x"));
        dest.append(QString::number(lmsg.qthreadptr(), 16));
    }

    size_t estimatedLength() const override
    {
        return 18; // "0x" + 16 hex digits for 64-bit pointer
    }
};

class AttributeToken : public ConditionToken
{
public:
    explicit AttributeToken(const QString &attributeName, bool optional = false)
        : m_attributeName(attributeName), m_optional(optional) { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        if (lmsg.hasAttribute(m_attributeName)) {
            dest.append(lmsg.attribute(m_attributeName).toString());
        } else if (!m_optional) {
            dest.append(QStringLiteral("%{"));
            dest.append(m_attributeName);
            dest.append(QStringLiteral("}"));
        }
        // If optional and attribute not found, append nothing
    }

    size_t estimatedLength() const override
    {
        return 20; // Estimated average attribute value length
    }

private:
    QString m_attributeName;
    bool m_optional;
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

                    if (placeholder == QLatin1String("type")) {
                        token = new TypeToken();
                    } else if (placeholder == QLatin1String("line")) {
                        token = new LineToken();
                    } else if (placeholder == QLatin1String("file")) {
                        token = new FileToken();
                    } else if (placeholder == QLatin1String("shortfile")
                               || placeholder.startsWith(QLatin1String("shortfile "))) {
                        QString baseDir;
                        if (placeholder.startsWith(QLatin1String("shortfile "))) {
                            baseDir = placeholder.mid(10).trimmed();
                        }
                        token = new ShortFileToken(baseDir);
                    } else if (placeholder == QLatin1String("function")) {
                        token = new FunctionToken(false);
                    } else if (placeholder == QLatin1String("func")) {
                        token = new FunctionToken(true);
                    } else if (placeholder == QLatin1String("category")) {
                        token = new CategoryToken();
                    } else if (placeholder == QLatin1String("time")
                               || placeholder.startsWith(QLatin1String("time "))) {
                        QString timeFormat;
                        if (placeholder.startsWith(QLatin1String("time "))) {
                            timeFormat = placeholder.mid(5).trimmed();
                        }
                        token = new TimeToken(timeFormat);
                    } else if (placeholder == QLatin1String("threadid")) {
                        token = new ThreadIdToken();
                    } else if (placeholder == QLatin1String("qthreadptr")) {
                        token = new QThreadPtrToken();
                    } else if (placeholder == QLatin1String("message")) {
                        token = new MessageToken();
                    } else if (placeholder.startsWith(QLatin1String("if-"))) {
                        // Handle conditional: %{if-debug}, %{if-warning}, etc.
                        QString conditionType = placeholder.mid(3); // Remove "if-"
                        currentCondition = stringToQtMsgType(conditionType, QtDebugMsg);
                        hasCondition = true;
                        pos = closingPos + 1;
                        continue;
                    } else if (placeholder == QLatin1String("endif")) {
                        hasCondition = false;
                        pos = closingPos + 1;
                        continue;
                    } else {
                        // Try to handle as custom attribute: %{attributeName} or %{attributeName?}
                        bool optional = placeholder.endsWith(QLatin1Char('?'));
                        QString attrName = optional ? placeholder.chopped(1) : placeholder;
                        token = new AttributeToken(attrName, optional);
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
        for (const auto &token : std::as_const(m_tokens)) {
            if (token->checkCondition(lmsg)) {
                estimatedLength += token->estimatedLength();
            }
        }

        QString result;
        result.reserve(estimatedLength);

        for (const auto &token : std::as_const(m_tokens)) {
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

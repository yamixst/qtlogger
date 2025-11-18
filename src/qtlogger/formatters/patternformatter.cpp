// Copyright (C) 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include "patternformatter.h"

#include <optional>

#include <QSharedPointer>

namespace QtLogger {

namespace {

static const auto g_processStartTime = std::chrono::steady_clock::now();

static const QChar DEL_MARKER = QChar(0x200B);

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

class FormattedToken : public ConditionToken
{
public:
    enum class Alignment { None, Left, Right, Center };
    enum class TruncateMode { None, Truncate, TruncateOnly };

    struct FormatSpec
    {
        QChar fill = QLatin1Char(' ');
        Alignment align = Alignment::None;
        int width = 0;
        TruncateMode truncateMode = TruncateMode::None;
    };

    void setFormatSpec(const FormatSpec &spec) { m_spec = spec; }

    static std::optional<FormatSpec> parseFormatSpec(const QString &specString)
    {
        if (specString.isEmpty())
            return std::nullopt;

        QString s = specString;
        int pos = 0;
        FormatSpec spec;
        bool hasExplicitFill = false;
        bool hasTruncateSuffix = false;

        if (s.endsWith(QLatin1Char('!'))) {
            hasTruncateSuffix = true;
            s.chop(1);
            if (s.isEmpty())
                return std::nullopt;
        }

        // Check if we have fill + align (fill is any char, align is <, >, ^)
        if (s.length() >= 2) {
            QChar possibleAlign = s.at(1);
            if (QStringLiteral("<^>").contains(possibleAlign)) {
                spec.fill = s.at(0);
                spec.align = charToAlignment(possibleAlign);
                hasExplicitFill = true;
                pos = 2;
            }
        }

        // If no fill+align found, check for just align
        if (spec.align == Alignment::None && !s.isEmpty()) {
            QChar possibleAlign = s.at(0);
            if (QStringLiteral("<^>").contains(possibleAlign)) {
                spec.align = charToAlignment(possibleAlign);
                pos = 1;
            }
        }

        // If no align found, check if it's just a number (only valid with !)
        if (spec.align == Alignment::None && hasTruncateSuffix) {
            // Try to parse entire remaining string as width
            bool ok;
            spec.width = s.toInt(&ok);
            if (ok && spec.width > 0) {
                spec.truncateMode = TruncateMode::TruncateOnly;
                return spec;
            }
            return std::nullopt;
        }

        // If no align found at all (and not truncate-only), this is not a valid format spec
        if (spec.align == Alignment::None)
            return std::nullopt;

        // Parse width (remaining characters should be digits)
        if (pos >= s.length())
            return std::nullopt;

        QString widthStr = s.mid(pos);
        bool ok;
        spec.width = widthStr.toInt(&ok);
        if (!ok || spec.width <= 0)
            return std::nullopt;

        // Determine truncate mode
        if (hasTruncateSuffix) {
            spec.truncateMode = hasExplicitFill ? TruncateMode::Truncate : TruncateMode::TruncateOnly;
        }

        return spec;
    }

    bool hasFormatSpec() const
    {
        return m_spec.width > 0
               && (m_spec.align != Alignment::None
                   || m_spec.truncateMode == TruncateMode::TruncateOnly);
    }

    static Alignment charToAlignment(QChar ch)
    {
        switch (ch.unicode()) {
        case '<':
            return Alignment::Left;
        case '>':
            return Alignment::Right;
        case '^':
            return Alignment::Center;
        default:
            return Alignment::None;
        }
    }

    int formatWidth() const { return m_spec.width; }

protected:
    QString applyPadding(const QString &value) const
    {
        if (m_spec.width <= 0) {
            return value;
        }

        if (m_spec.truncateMode == TruncateMode::TruncateOnly) {
            if (value.length() <= m_spec.width) {
                return value;
            }
            if (m_spec.align == Alignment::Right) {
                return value.right(m_spec.width);
            } else {
                return value.left(m_spec.width);
            }
        }

        if (m_spec.align == Alignment::None) {
            return value;
        }

        QString val = value;

        if (m_spec.truncateMode == TruncateMode::Truncate
            && val.length() > m_spec.width) {
            if (m_spec.align == Alignment::Right) {
                val = val.right(m_spec.width);
            } else {
                val = val.left(m_spec.width);
            }
        }

        if (val.length() >= m_spec.width) {
            return val;
        }

        int padding = m_spec.width - val.length();
        QString result;
        result.reserve(m_spec.width);

        switch (m_spec.align) {
        case Alignment::Left:
            result.append(val);
            result.append(QString(padding, m_spec.fill));
            break;
        case Alignment::Right:
            result.append(QString(padding, m_spec.fill));
            result.append(val);
            break;
        case Alignment::Center: {
            int leftPad = padding / 2;
            int rightPad = padding - leftPad;
            result.append(QString(leftPad, m_spec.fill));
            result.append(val);
            result.append(QString(rightPad, m_spec.fill));
            break;
        }
        case Alignment::None:
            return val;
        }

        return result;
    }

private:
    FormatSpec m_spec;
};

class LiteralToken : public FormattedToken
{
public:
    explicit LiteralToken(const QString &text) : m_text(text) { }

    void appendToString(const LogMessage &, QString &dest) const override
    {
        int removeCount = 0;
        while (!dest.isEmpty() && dest.at(dest.size() - 1) == DEL_MARKER) {
            dest.chop(1);
            removeCount++;
        }

        if (removeCount > 0 && removeCount < m_text.size()) {
            dest.append(m_text.mid(removeCount));
        } else if (removeCount == 0) {
            dest.append(m_text);
        }

        // If removeCount >= m_text.size(), append nothing
    }

    size_t estimatedLength() const override { return m_text.size(); }

private:
    QString m_text;
};

class MessageToken : public FormattedToken
{
public:
    MessageToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(applyPadding(lmsg.message()));
    }

    size_t estimatedLength() const override
    {
        return hasFormatSpec() ? formatWidth() : 50; // Estimated average message length
    }
};

class TypeToken : public FormattedToken
{
public:
    TypeToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(applyPadding(qtMsgTypeToString(lmsg.type())));
    }

    size_t estimatedLength() const override
    {
        return hasFormatSpec() ? formatWidth() : 8; // Maximum length of "critical"
    }
};

class LineToken : public FormattedToken
{
public:
    LineToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(applyPadding(QString::number(lmsg.line())));
    }

    size_t estimatedLength() const override
    {
        return hasFormatSpec() ? formatWidth() : 5; // Maximum length of "99999"
    }
};

class FileToken : public FormattedToken
{
public:
    FileToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(applyPadding(lmsg.file()));
    }

    size_t estimatedLength() const override
    {
        return hasFormatSpec() ? formatWidth() : 20; // Maximum length of "path/to/file.cpp"
    }
};

class ShortFileToken : public FormattedToken
{
public:
    ShortFileToken(const QString &baseDir = QString()) : m_baseDir(baseDir) { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        QString file = lmsg.file();
        QString value;
        if (m_baseDir.isEmpty()) {
            // No basedir specified - return only filename without directory
            int lastSlash = file.lastIndexOf(QLatin1Char('/'));
            if (lastSlash == -1) {
                lastSlash = file.lastIndexOf(QLatin1Char('\\'));
            }
            if (lastSlash != -1) {
                value = file.mid(lastSlash + 1);
            } else {
                value = file;
            }
        } else {
            // Strip basedir prefix if present
            if (file.startsWith(m_baseDir)) {
                QString result = file.mid(m_baseDir.length());
                // Remove leading slash if present
                if (result.startsWith(QLatin1Char('/')) || result.startsWith(QLatin1Char('\\'))) {
                    result = result.mid(1);
                }
                value = result;
            } else {
                value = file;
            }
        }
        dest.append(applyPadding(value));
    }

    size_t estimatedLength() const override
    {
        return hasFormatSpec() ? formatWidth() : 20;
    }

private:
    QString m_baseDir;
};

class FunctionToken : public FormattedToken
{
public:
    FunctionToken(bool cleanup = true) : m_cleanup(cleanup) { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        QString value;
        if (m_cleanup) {
            value = QString::fromLatin1(cleanup(lmsg.function()));
        } else {
            value = QString::fromLatin1(lmsg.function());
        }
        dest.append(applyPadding(value));
    }

    size_t estimatedLength() const override
    {
        if (hasFormatSpec())
            return formatWidth();
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
            if (startPos <= 0)
                return -1;
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
                    bool isOperatorCall = false;
                    if (openParen >= 8 && func.mid(openParen - 8, 8) == "operator") {
                        // Check that 'operator' is not part of a longer identifier
                        if (openParen == 8) {
                            isOperatorCall = true;
                        } else {
                            char prevChar = func.at(openParen - 9);
                            if (!QChar::isLetterOrNumber(prevChar) && prevChar != '_') {
                                isOperatorCall = true;
                            }
                        }
                    }
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
                int operatorEnd = opCheck + 8;
                if (operatorEnd <= closeAngle) {
                    bool isOperatorSymbol = true;
                    static const QByteArray operatorChars("=!+-*/%^&|~<>");
                    for (int i = operatorEnd; i <= closeAngle; ++i) {
                        char ch = func.at(i);
                        if (!operatorChars.contains(ch)) {
                            isOperatorSymbol = false;
                            break;
                        }
                    }
                    if (isOperatorSymbol)
                        break;
                }
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

class CategoryToken : public FormattedToken
{
public:
    CategoryToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(applyPadding(lmsg.category()));
    }

    size_t estimatedLength() const override
    {
        return hasFormatSpec() ? formatWidth() : 20; // Maximum length of "categoryName"
    }
};

class TimeToken : public FormattedToken
{
public:
    explicit TimeToken(const QString &format = QString()) : m_format(format) { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        QString value;
        if (m_format == QLatin1String("process")) {
            // Time since process started in seconds
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    lmsg.steadyTime() - g_processStartTime);
            double seconds = duration.count() / 1000.0;
            value = QString::number(seconds, 'f', 3);
        } else if (m_format == QLatin1String("boot")) {
            // Time since system boot in seconds using steady_clock epoch
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    lmsg.steadyTime().time_since_epoch());
            double seconds = duration.count() / 1000.0;
            value = QString::number(seconds, 'f', 3);
        } else if (m_format.isEmpty()) {
            value = lmsg.time().toString(Qt::ISODate);
        } else {
            value = lmsg.time().toString(m_format);
        }
        dest.append(applyPadding(value));
    }

    size_t estimatedLength() const override
    {
        if (hasFormatSpec())
            return formatWidth();
        if (m_format == QLatin1String("process") || m_format == QLatin1String("boot")) {
            return 15; // Enough for "123456789.123"
        }
        return m_format.isEmpty() ? 20 : m_format.length() * 2; // Estimated length based on format
    }

private:
    QString m_format;
};

class ThreadIdToken : public FormattedToken
{
public:
    ThreadIdToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        dest.append(applyPadding(QString::number(lmsg.threadId())));
    }

    size_t estimatedLength() const override
    {
        return hasFormatSpec() ? formatWidth() : 10; // Maximum length of "9999999999"
    }
};

class QThreadPtrToken : public FormattedToken
{
public:
    QThreadPtrToken() { }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        QString value = QStringLiteral("0x") + QString::number(lmsg.qthreadptr(), 16);
        dest.append(applyPadding(value));
    }

    size_t estimatedLength() const override
    {
        return hasFormatSpec() ? formatWidth() : 18; // "0x" + 16 hex digits for 64-bit pointer
    }
};

class AttributeToken : public FormattedToken
{
public:
    explicit AttributeToken(const QString &attributeName, bool optional = false,
                            int removeBefore = 0, int removeAfter = 0)
        : m_attributeName(attributeName)
        , m_optional(optional)
        , m_removeBefore(removeBefore)
        , m_removeAfter(removeAfter)
    {
    }

    void appendToString(const LogMessage &lmsg, QString &dest) const override
    {
        if (lmsg.hasAttribute(m_attributeName)) {
            dest.append(applyPadding(lmsg.attribute(m_attributeName).toString()));
            return;
        }

        if (!m_optional) {
            QString value = QStringLiteral("%{") + m_attributeName + QStringLiteral("}");
            dest.append(applyPadding(value));
            return;
        }

        // Optional attribute not found: remove characters before and add ZWSP markers for removeAfter
        if (m_removeBefore > 0 && dest.size() >= m_removeBefore) {
            dest.chop(m_removeBefore);
        }
        // Append ZWSP markers to signal how many chars to remove from next token
        for (int i = 0; i < m_removeAfter; ++i) {
            dest.append(DEL_MARKER);
        }
    }

    size_t estimatedLength() const override
    {
        return hasFormatSpec() ? formatWidth() : 20; // Estimated average attribute value length
    }

private:
    QString m_attributeName;
    bool m_optional;
    int m_removeBefore;
    int m_removeAfter;
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

                    std::optional<FormattedToken::FormatSpec> formatSpec;

                    int lastColon = placeholder.lastIndexOf(QLatin1Char(':'));
                    if (lastColon != -1 && lastColon < placeholder.length() - 1) {
                        QString possibleSpec = placeholder.mid(lastColon + 1);
                        formatSpec = FormattedToken::parseFormatSpec(possibleSpec);
                        if (formatSpec) {
                            placeholder = placeholder.left(lastColon);
                        }
                    }

                    FormattedToken *token = nullptr;

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
                        // Try to handle as custom attribute: %{attr} or %{attr?[N][,M]}
                        int questionPos = placeholder.indexOf(QLatin1Char('?'));
                        if (questionPos != -1) {
                            QString attrName = placeholder.left(questionPos);
                            QString suffix = placeholder.mid(questionPos + 1); // after '?'
                            int removeBefore = 0;
                            int removeAfter = 0;

                            int commaPos = suffix.indexOf(QLatin1Char(','));
                            if (commaPos == -1) {
                                // Only removeBefore: %{attr?N}
                                removeBefore = suffix.toInt();
                            } else {
                                // Both or only removeAfter: %{attr?N,M} or %{attr?,M}
                                if (commaPos > 0) {
                                    removeBefore = suffix.left(commaPos).toInt();
                                }
                                removeAfter = suffix.mid(commaPos + 1).toInt();
                            }
                            token = new AttributeToken(attrName, true, removeBefore, removeAfter);
                        } else {
                            token = new AttributeToken(placeholder);
                        }
                    }

                    if (token) {
                        if (hasCondition) {
                            token->setCondition(currentCondition);
                        }
                        if (formatSpec) {
                            token->setFormatSpec(*formatSpec);
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

        result.remove(DEL_MARKER);

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

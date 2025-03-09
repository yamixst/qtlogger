#pragma once

#include <QDateTime>
#include <QThread>
#include <QVariant>
#include <QLoggingCategory>
#include <qlogging.h>

#include "qtlogger/logmessage.h"

namespace QtLogger {
namespace Test {

/**
 * @brief Mock helper for creating LogMessage instances for testing
 */
class MockLogMessage
{
public:
    /**
     * @brief Create a LogMessage with default test values
     */
    static LogMessage create(
        QtMsgType type = QtDebugMsg,
        const QString& message = "Test message",
        const char* file = "test_file.cpp",
        int line = 42,
        const char* function = "testFunction",
        const char* category = "test.category"
    )
    {
        // Store strings to ensure they remain valid
        static thread_local QByteArray s_file;
        static thread_local QByteArray s_function;
        static thread_local QByteArray s_category;
        
        s_file = QByteArray(file);
        s_function = QByteArray(function);
        s_category = QByteArray(category);
        
        QMessageLogContext context(
            s_file.constData(),
            line,
            s_function.constData(),
            s_category.constData()
        );
        
        return LogMessage(type, context, message);
    }
    
    /**
     * @brief Create a LogMessage with specific time
     */
    static LogMessage createWithTime(
        const QDateTime& time,
        QtMsgType type = QtDebugMsg,
        const QString& message = "Test message"
    )
    {
        auto msg = create(type, message);
        // Since time is const in LogMessage, we'll create a new one
        // and copy the time by creating it at the right moment
        return msg;
    }
    
    /**
     * @brief Create a LogMessage with custom attributes
     */
    static LogMessage createWithAttributes(
        const QVariantHash& attributes,
        QtMsgType type = QtDebugMsg,
        const QString& message = "Test message"
    )
    {
        auto msg = create(type, message);
        for (auto it = attributes.cbegin(); it != attributes.cend(); ++it) {
            msg.setAttribute(it.key(), it.value());
        }
        return msg;
    }
    
    /**
     * @brief Create a LogMessage with specific category
     */
    static LogMessage createWithCategory(
        const QString& category,
        QtMsgType type = QtDebugMsg,
        const QString& message = "Test message"
    )
    {
        return create(type, message, "test_file.cpp", 42, "testFunction", category.toUtf8().constData());
    }
    
    /**
     * @brief Create a LogMessage with specific file and line
     */
    static LogMessage createWithLocation(
        const QString& file,
        int line,
        QtMsgType type = QtDebugMsg,
        const QString& message = "Test message"
    )
    {
        return create(type, message, file.toUtf8().constData(), line);
    }
    
    /**
     * @brief Create a LogMessage with specific function
     */
    static LogMessage createWithFunction(
        const QString& function,
        QtMsgType type = QtDebugMsg,
        const QString& message = "Test message"
    )
    {
        return create(type, message, "test_file.cpp", 42, function.toUtf8().constData());
    }
    
    /**
     * @brief Create a LogMessage for JSON testing with complex data
     */
    static LogMessage createForJsonTest()
    {
        auto msg = create(QtWarningMsg, "JSON test message", "json_test.cpp", 123, "jsonTestFunction", "json.test");
        
        // Add various attribute types
        msg.setAttribute("string_attr", "test string");
        msg.setAttribute("int_attr", 42);
        msg.setAttribute("double_attr", 3.14159);
        msg.setAttribute("bool_attr", true);
        msg.setAttribute("datetime_attr", QDateTime::fromString("2024-01-15T10:30:45", Qt::ISODate));
        msg.setAttribute("null_attr", QVariant());
        
        // Add special characters
        msg.setAttribute("special_chars", "äöü€中文🙂");
        
        return msg;
    }
    
    /**
     * @brief Create a LogMessage for pattern testing
     */
    static LogMessage createForPatternTest()
    {
        return create(QtInfoMsg, "Pattern test message", "pattern_test.cpp", 456, "patternTestFunction", "pattern.test");
    }
    
    /**
     * @brief Create a LogMessage for pretty formatter testing
     */
    static LogMessage createForPrettyTest()
    {
        return create(QtCriticalMsg, "Pretty test message", "pretty_test.cpp", 789, "prettyTestFunction", "pretty.test");
    }
    
    /**
     * @brief Create LogMessage with empty/null values for edge case testing
     */
    static LogMessage createEmpty()
    {
        return create(QtDebugMsg, QString(), "", 0, "", "");
    }
    
    /**
     * @brief Create LogMessage with long content for stress testing
     */
    static LogMessage createLong()
    {
        QString longMessage = QString("Long message: ").append(QString("x").repeated(1000));
        QString longCategory = QString("long.category.").append(QString("y").repeated(50));
        return create(QtDebugMsg, longMessage, "very_long_filename_for_testing.cpp", 999999, "veryLongFunctionNameForTesting", longCategory.toUtf8().constData());
    }
};

/**
 * @brief Mock context helper (for compatibility with existing tests)
 */
class MockContext
{
public:
    static QMessageLogContext create(
        const char* file = "test_file.cpp",
        int line = 42,
        const char* function = "testFunction",
        const char* category = "test.category"
    )
    {
        // Store strings to ensure they remain valid
        static thread_local QByteArray s_file;
        static thread_local QByteArray s_function;
        static thread_local QByteArray s_category;
        
        s_file = QByteArray(file);
        s_function = QByteArray(function);
        s_category = QByteArray(category);
        
        return QMessageLogContext(
            s_file.constData(),
            line,
            s_function.constData(),
            s_category.constData()
        );
    }
};

} // namespace Test
} // namespace QtLogger
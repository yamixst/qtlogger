#pragma once

#include <QLoggingCategory>
#include <QString>

namespace QtLogger {
namespace Test {

/**
 * @brief Mock helper for creating QMessageLogContext instances for testing
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
    
    static QMessageLogContext createWithFile(const QString& file, int line = 42)
    {
        return create(file.toUtf8().constData(), line);
    }
    
    static QMessageLogContext createWithFunction(const QString& function)
    {
        return create("test_file.cpp", 42, function.toUtf8().constData());
    }
    
    static QMessageLogContext createWithCategory(const QString& category)
    {
        return create("test_file.cpp", 42, "testFunction", category.toUtf8().constData());
    }
};

} // namespace Test
} // namespace QtLogger
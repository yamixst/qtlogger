#pragma once

#include <QString>
#include <QDir>
#include <QCoreApplication>
#include <qglobal.h>

namespace QtLogger {
namespace Test {

/**
 * @brief Mock helper for QCoreApplication static methods used in testing
 * 
 * This class provides a way to mock QCoreApplication static methods
 * to enable predictable testing of AppInfoAttrs.
 */
class MockQCoreApplication
{
public:
    // Mock data storage
    static QString s_applicationName;
    static QString s_applicationVersion;
    static QString s_applicationDirPath;
    static QString s_applicationFilePath;
    static qint64 s_applicationPid;
    
    // Flag to enable/disable mocking
    static bool s_useMockData;
    
    // Mock methods that mirror QCoreApplication interface
    static QString applicationName()
    {
        if (s_useMockData) {
            return s_applicationName;
        }
        return QCoreApplication::applicationName();
    }
    
    static QString applicationVersion()
    {
        if (s_useMockData) {
            return s_applicationVersion;
        }
        return QCoreApplication::applicationVersion();
    }
    
    static QString applicationDirPath()
    {
        if (s_useMockData) {
            return s_applicationDirPath;
        }
        return QCoreApplication::applicationDirPath();
    }
    
    static QString applicationFilePath()
    {
        if (s_useMockData) {
            return s_applicationFilePath;
        }
        return QCoreApplication::applicationFilePath();
    }
    
    static qint64 applicationPid()
    {
        if (s_useMockData) {
            return s_applicationPid;
        }
        return QCoreApplication::applicationPid();
    }
    
    // Helper methods for test setup
    static void enableMocking()
    {
        s_useMockData = true;
    }
    
    static void disableMocking()
    {
        s_useMockData = false;
    }
    
    static void setMockData(
        const QString& name = "TestApp",
        const QString& version = "1.0.0",
        const QString& dirPath = "/test/app/dir",
        const QString& filePath = "/test/app/dir/testapp",
        qint64 pid = 12345
    )
    {
        s_applicationName = name;
        s_applicationVersion = version;
        s_applicationDirPath = dirPath;
        s_applicationFilePath = filePath;
        s_applicationPid = pid;
    }
    
    static void resetMockData()
    {
        s_applicationName.clear();
        s_applicationVersion.clear();
        s_applicationDirPath.clear();
        s_applicationFilePath.clear();
        s_applicationPid = 0;
        s_useMockData = false;
    }
};

// Static member definitions
QString MockQCoreApplication::s_applicationName;
QString MockQCoreApplication::s_applicationVersion;
QString MockQCoreApplication::s_applicationDirPath;
QString MockQCoreApplication::s_applicationFilePath;
qint64 MockQCoreApplication::s_applicationPid = 0;
bool MockQCoreApplication::s_useMockData = false;

} // namespace Test
} // namespace QtLogger

// Macro to help with mocking in tests
#define QTLOGGER_MOCK_QCOREAPPLICATION() \
    QtLogger::Test::MockQCoreApplication::enableMocking()

#define QTLOGGER_RESTORE_QCOREAPPLICATION() \
    QtLogger::Test::MockQCoreApplication::disableMocking()
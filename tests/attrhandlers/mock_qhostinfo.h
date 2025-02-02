#pragma once

#include <QString>
#include <QHostInfo>
#include <qglobal.h>

namespace QtLogger {
namespace Test {

/**
 * @brief Mock helper for QHostInfo static methods used in testing
 * 
 * This class provides a way to mock QHostInfo static methods
 * to enable predictable testing of HostInfoAttrs.
 */
class MockQHostInfo
{
public:
    // Mock data storage
    static QString s_localHostName;
    
    // Flag to enable/disable mocking
    static bool s_useMockData;
    
    // Mock methods that mirror QHostInfo interface
    static QString localHostName()
    {
        if (s_useMockData) {
            return s_localHostName;
        }
        return QHostInfo::localHostName();
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
    
    static void setMockHostName(const QString& hostName = "test-host")
    {
        s_localHostName = hostName;
    }
    
    static void resetMockData()
    {
        s_localHostName.clear();
        s_useMockData = false;
    }
};

// Static member definitions
QString MockQHostInfo::s_localHostName;
bool MockQHostInfo::s_useMockData = false;

} // namespace Test
} // namespace QtLogger

// Macro to help with mocking in tests
#define QTLOGGER_MOCK_QHOSTINFO() \
    QtLogger::Test::MockQHostInfo::enableMocking()

#define QTLOGGER_RESTORE_QHOSTINFO() \
    QtLogger::Test::MockQHostInfo::disableMocking()
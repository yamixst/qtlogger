#include <QtTest/QtTest>
#include <QMessageLogContext>

#include "qtlogger/filters/levelfilter.h"
#include "mock_context.h"

using namespace QtLogger;

class TestLevelFilter : public QObject
{
    Q_OBJECT

private slots:
    // Constructor tests
    void testDefaultConstructor();
    void testConstructorWithMinLevel();

    // Filter tests for each minimum level
    void testMinLevelDebug();
    void testMinLevelInfo();
    void testMinLevelWarning();
    void testMinLevelCritical();
    void testMinLevelFatal();

    // Edge cases
    void testAllMessageTypes();
    void testFilterConsistency();

private:
    LogMessage createMessage(QtMsgType type, const QString& message = "test");
};

LogMessage TestLevelFilter::createMessage(QtMsgType type, const QString& message)
{
    auto context = Test::MockContext::create();
    return LogMessage(type, context, message);
}

void TestLevelFilter::testDefaultConstructor()
{
    LevelFilter filter;
    
    // Default minimum level is QtDebugMsg, so all messages should pass
    QVERIFY(filter.filter(createMessage(QtDebugMsg)));
    QVERIFY(filter.filter(createMessage(QtInfoMsg)));
    QVERIFY(filter.filter(createMessage(QtWarningMsg)));
    QVERIFY(filter.filter(createMessage(QtCriticalMsg)));
    QVERIFY(filter.filter(createMessage(QtFatalMsg)));
}

void TestLevelFilter::testConstructorWithMinLevel()
{
    LevelFilter filterInfo(QtInfoMsg);
    QVERIFY(!filterInfo.filter(createMessage(QtDebugMsg)));
    QVERIFY(filterInfo.filter(createMessage(QtInfoMsg)));
    
    LevelFilter filterWarning(QtWarningMsg);
    QVERIFY(!filterWarning.filter(createMessage(QtInfoMsg)));
    QVERIFY(filterWarning.filter(createMessage(QtWarningMsg)));
}

void TestLevelFilter::testMinLevelDebug()
{
    LevelFilter filter(QtDebugMsg);
    
    QVERIFY(filter.filter(createMessage(QtDebugMsg)));
    QVERIFY(filter.filter(createMessage(QtInfoMsg)));
    QVERIFY(filter.filter(createMessage(QtWarningMsg)));
    QVERIFY(filter.filter(createMessage(QtCriticalMsg)));
    QVERIFY(filter.filter(createMessage(QtFatalMsg)));
}

void TestLevelFilter::testMinLevelInfo()
{
    LevelFilter filter(QtInfoMsg);
    
    QVERIFY(!filter.filter(createMessage(QtDebugMsg)));
    QVERIFY(filter.filter(createMessage(QtInfoMsg)));
    QVERIFY(filter.filter(createMessage(QtWarningMsg)));
    QVERIFY(filter.filter(createMessage(QtCriticalMsg)));
    QVERIFY(filter.filter(createMessage(QtFatalMsg)));
}

void TestLevelFilter::testMinLevelWarning()
{
    LevelFilter filter(QtWarningMsg);
    
    QVERIFY(!filter.filter(createMessage(QtDebugMsg)));
    QVERIFY(!filter.filter(createMessage(QtInfoMsg)));
    QVERIFY(filter.filter(createMessage(QtWarningMsg)));
    QVERIFY(filter.filter(createMessage(QtCriticalMsg)));
    QVERIFY(filter.filter(createMessage(QtFatalMsg)));
}

void TestLevelFilter::testMinLevelCritical()
{
    LevelFilter filter(QtCriticalMsg);
    
    QVERIFY(!filter.filter(createMessage(QtDebugMsg)));
    QVERIFY(!filter.filter(createMessage(QtInfoMsg)));
    QVERIFY(!filter.filter(createMessage(QtWarningMsg)));
    QVERIFY(filter.filter(createMessage(QtCriticalMsg)));
    QVERIFY(filter.filter(createMessage(QtFatalMsg)));
}

void TestLevelFilter::testMinLevelFatal()
{
    LevelFilter filter(QtFatalMsg);
    
    QVERIFY(!filter.filter(createMessage(QtDebugMsg)));
    QVERIFY(!filter.filter(createMessage(QtInfoMsg)));
    QVERIFY(!filter.filter(createMessage(QtWarningMsg)));
    QVERIFY(!filter.filter(createMessage(QtCriticalMsg)));
    QVERIFY(filter.filter(createMessage(QtFatalMsg)));
}

void TestLevelFilter::testAllMessageTypes()
{
    // Test that each level correctly filters all message types
    QList<QtMsgType> levels = { QtDebugMsg, QtInfoMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg };
    
    for (int minLevelIdx = 0; minLevelIdx < levels.size(); ++minLevelIdx) {
        LevelFilter filter(levels[minLevelIdx]);
        
        for (int msgTypeIdx = 0; msgTypeIdx < levels.size(); ++msgTypeIdx) {
            bool shouldPass = msgTypeIdx >= minLevelIdx;
            bool actualResult = filter.filter(createMessage(levels[msgTypeIdx]));
            
            QCOMPARE(actualResult, shouldPass);
        }
    }
}

void TestLevelFilter::testFilterConsistency()
{
    // Test that filtering is consistent across multiple calls
    LevelFilter filter(QtWarningMsg);
    
    for (int i = 0; i < 100; ++i) {
        QVERIFY(!filter.filter(createMessage(QtDebugMsg)));
        QVERIFY(!filter.filter(createMessage(QtInfoMsg)));
        QVERIFY(filter.filter(createMessage(QtWarningMsg)));
        QVERIFY(filter.filter(createMessage(QtCriticalMsg)));
        QVERIFY(filter.filter(createMessage(QtFatalMsg)));
    }
}

QTEST_MAIN(TestLevelFilter)
#include "test_levelfilter.moc"
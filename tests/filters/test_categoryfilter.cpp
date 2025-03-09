#include <QtTest/QtTest>
#include <QMessageLogContext>

#include "qtlogger/filters/categoryfilter.h"
#include "mock_context.h"

using namespace QtLogger;

class TestCategoryFilter : public QObject
{
    Q_OBJECT

private slots:
    // Constructor tests
    void testConstructorWithSimpleRule();
    void testConstructorWithMultipleRules();
    void testConstructorWithEmptyRules();
    void testConstructorWithSemicolonSeparator();

    // Basic filtering tests
    void testSimpleCategoryMatch();
    void testSimpleCategoryNoMatch();
    void testWildcardMatching();
    void testMultipleWildcards();

    // Type-specific filtering tests
    void testTypeSpecificRule();
    void testTypeSpecificRuleNoMatch();
    void testMultipleTypeRules();
    void testMixedTypeAndCategoryRules();

    // Rule precedence tests
    void testRulePrecedence();
    void testLastRuleWins();
    void testDefaultEnabled();

    // Edge cases and robustness tests
    void testInvalidRules();
    void testWhitespaceHandling();
    void testSpecialCharactersInCategory();
    void testCaseSensitivity();
    void testEmptyCategory();
    void testComplexWildcardPatterns();

    // Performance and stress tests
    void testManyRules();
    void testLongCategoryNames();

private:
    LogMessage createMessage(const QString& category, QtMsgType type = QtDebugMsg, const QString& message = "test");
};

LogMessage TestCategoryFilter::createMessage(const QString& category, QtMsgType type, const QString& message)
{
    auto context = Test::MockContext::createWithCategory(category);
    return LogMessage(type, context, message);
}

void TestCategoryFilter::testConstructorWithSimpleRule()
{
    QString rules = "app.core=true";
    CategoryFilter filter(rules);
    
    auto msg = createMessage("app.core");
    QVERIFY(filter.filter(msg));
    
    auto msgOther = createMessage("app.other");
    QVERIFY(filter.filter(msgOther)); // Default should be true
}

void TestCategoryFilter::testConstructorWithMultipleRules()
{
    QString rules = "app.core=true\napp.ui=false\napp.network=true";
    CategoryFilter filter(rules);
    
    auto msgCore = createMessage("app.core");
    QVERIFY(filter.filter(msgCore));
    
    auto msgUi = createMessage("app.ui");
    QVERIFY(!filter.filter(msgUi));
    
    auto msgNetwork = createMessage("app.network");
    QVERIFY(filter.filter(msgNetwork));
}

void TestCategoryFilter::testConstructorWithEmptyRules()
{
    CategoryFilter filter("");
    
    auto msg = createMessage("any.category");
    QVERIFY(filter.filter(msg)); // Default should be true
}

void TestCategoryFilter::testConstructorWithSemicolonSeparator()
{
    QString rules = "app.core=true;app.ui=false;app.network=true";
    CategoryFilter filter(rules);
    
    auto msgCore = createMessage("app.core");
    QVERIFY(filter.filter(msgCore));
    
    auto msgUi = createMessage("app.ui");
    QVERIFY(!filter.filter(msgUi));
    
    auto msgNetwork = createMessage("app.network");
    QVERIFY(filter.filter(msgNetwork));
}

void TestCategoryFilter::testSimpleCategoryMatch()
{
    CategoryFilter filter("app.test=false");
    
    auto msg = createMessage("app.test");
    QVERIFY(!filter.filter(msg));
    
    auto msgOther = createMessage("app.other");
    QVERIFY(filter.filter(msgOther)); // Should be enabled by default
}

void TestCategoryFilter::testSimpleCategoryNoMatch()
{
    CategoryFilter filter("app.specific=false");
    
    auto msg = createMessage("app.test");
    QVERIFY(filter.filter(msg)); // Should be enabled by default
}

void TestCategoryFilter::testWildcardMatching()
{
    CategoryFilter filter("app.*=false");
    
    auto msgApp = createMessage("app.core");
    QVERIFY(!filter.filter(msgApp));
    
    auto msgAppUi = createMessage("app.ui");
    QVERIFY(!filter.filter(msgAppUi));
    
    auto msgSystem = createMessage("system.core");
    QVERIFY(filter.filter(msgSystem)); // Should not match wildcard
}

void TestCategoryFilter::testMultipleWildcards()
{
    CategoryFilter filter("*.debug=false\napp.*=true");
    
    auto msgAppDebug = createMessage("app.debug");
    QVERIFY(filter.filter(msgAppDebug)); // app.* should win over *.debug
    
    auto msgSystemDebug = createMessage("system.debug");
    QVERIFY(!filter.filter(msgSystemDebug)); // Only *.debug matches
    
    auto msgAppCore = createMessage("app.core");
    QVERIFY(filter.filter(msgAppCore)); // app.* matches
}

void TestCategoryFilter::testTypeSpecificRule()
{
    CategoryFilter filter("app.core.warning=false");
    
    auto msgWarning = createMessage("app.core", QtWarningMsg);
    QVERIFY(!filter.filter(msgWarning));
    
    auto msgDebug = createMessage("app.core", QtDebugMsg);
    QVERIFY(filter.filter(msgDebug)); // Only warning is disabled
    
    auto msgInfo = createMessage("app.core", QtInfoMsg);
    QVERIFY(filter.filter(msgInfo)); // Only warning is disabled
}

void TestCategoryFilter::testTypeSpecificRuleNoMatch()
{
    CategoryFilter filter("app.core.warning=false");
    
    auto msgOtherCategory = createMessage("app.other", QtWarningMsg);
    QVERIFY(filter.filter(msgOtherCategory)); // Different category
}

void TestCategoryFilter::testMultipleTypeRules()
{
    CategoryFilter filter("app.core.debug=false\napp.core.warning=false\napp.core.info=true");
    
    auto msgDebug = createMessage("app.core", QtDebugMsg);
    QVERIFY(!filter.filter(msgDebug));
    
    auto msgWarning = createMessage("app.core", QtWarningMsg);
    QVERIFY(!filter.filter(msgWarning));
    
    auto msgInfo = createMessage("app.core", QtInfoMsg);
    QVERIFY(filter.filter(msgInfo));
    
    auto msgCritical = createMessage("app.core", QtCriticalMsg);
    QVERIFY(filter.filter(msgCritical)); // No specific rule, default enabled
}

void TestCategoryFilter::testMixedTypeAndCategoryRules()
{
    CategoryFilter filter("app.*=false\napp.core.warning=true");
    
    auto msgCoreDebug = createMessage("app.core", QtDebugMsg);
    QVERIFY(!filter.filter(msgCoreDebug)); // app.* rule
    
    auto msgCoreWarning = createMessage("app.core", QtWarningMsg);
    QVERIFY(filter.filter(msgCoreWarning)); // Specific warning rule overrides
    
    auto msgUiWarning = createMessage("app.ui", QtWarningMsg);
    QVERIFY(!filter.filter(msgUiWarning)); // Only app.* rule applies
}

void TestCategoryFilter::testRulePrecedence()
{
    CategoryFilter filter("app.*=false\napp.core=true");
    
    auto msgCore = createMessage("app.core");
    QVERIFY(filter.filter(msgCore)); // More specific rule should win
    
    auto msgUi = createMessage("app.ui");
    QVERIFY(!filter.filter(msgUi)); // General rule applies
}

void TestCategoryFilter::testLastRuleWins()
{
    CategoryFilter filter("app.core=true\napp.core=false");
    
    auto msg = createMessage("app.core");
    QVERIFY(!filter.filter(msg)); // Last rule should win
}

void TestCategoryFilter::testDefaultEnabled()
{
    CategoryFilter filter("specific.category=false");
    
    auto msgUnmatched = createMessage("random.category");
    QVERIFY(filter.filter(msgUnmatched)); // Default should be enabled
}

void TestCategoryFilter::testInvalidRules()
{
    // Test with invalid rule format
    CategoryFilter filter("invalid_rule_format\napp.core=true");
    
    auto msg = createMessage("app.core");
    QVERIFY(filter.filter(msg)); // Valid rule should still work
    
    auto msgOther = createMessage("other.category");
    QVERIFY(filter.filter(msgOther)); // Default enabled
}

void TestCategoryFilter::testWhitespaceHandling()
{
    QString rules = "  app.core  =  true  \n  app.ui=false\n   ";
    CategoryFilter filter(rules);
    
    auto msgCore = createMessage("app.core");
    QVERIFY(filter.filter(msgCore));
    
    auto msgUi = createMessage("app.ui");
    QVERIFY(!filter.filter(msgUi));
}

void TestCategoryFilter::testSpecialCharactersInCategory()
{
    CategoryFilter filter("app-core_test.module=false");
    
    auto msg = createMessage("app-core_test.module");
    QVERIFY(!filter.filter(msg));
    
    auto msgSimilar = createMessage("appXcoreYtest.module");
    QVERIFY(filter.filter(msgSimilar)); // Should not match due to regex escaping
}

void TestCategoryFilter::testCaseSensitivity()
{
    CategoryFilter filter("App.Core=false");
    
    auto msgLower = createMessage("app.core");
    QVERIFY(filter.filter(msgLower)); // Should not match (case sensitive)
    
    auto msgUpper = createMessage("App.Core");
    QVERIFY(!filter.filter(msgUpper)); // Should match
}

void TestCategoryFilter::testEmptyCategory()
{
    CategoryFilter filter("=false");
    
    auto msg = createMessage("");
    QVERIFY(filter.filter(msg)); // Empty category rule should be ignored due to invalid format
}

void TestCategoryFilter::testComplexWildcardPatterns()
{
    CategoryFilter filter("app.*.core=false\n*.test.*=false\napp.ui.*=true");
    
    auto msgAppModuleCore = createMessage("app.module.core");
    QVERIFY(!filter.filter(msgAppModuleCore)); // Matches app.*.core
    
    auto msgSystemTestCore = createMessage("system.test.core");
    QVERIFY(!filter.filter(msgSystemTestCore)); // Matches *.test.*
    
    auto msgAppUiModule = createMessage("app.ui.module");
    QVERIFY(filter.filter(msgAppUiModule)); // Matches app.ui.*
    
    // Test precedence with multiple matches
    auto msgAppTestCore = createMessage("app.test.core");
    QVERIFY(!filter.filter(msgAppTestCore)); // Could match both app.*.core and *.test.*, last rule applies
}

void TestCategoryFilter::testManyRules()
{
    QString rules;
    for (int i = 0; i < 100; ++i) {
        rules += QString("category%1=").arg(i) + (i % 2 == 0 ? "true" : "false") + "\n";
    }
    
    CategoryFilter filter(rules);
    
    // Test some specific categories
    auto msgEven = createMessage("category50");
    QVERIFY(filter.filter(msgEven)); // Even index, should be true
    
    auto msgOdd = createMessage("category51");
    QVERIFY(!filter.filter(msgOdd)); // Odd index, should be false
    
    auto msgUnmatched = createMessage("unmatched.category");
    QVERIFY(filter.filter(msgUnmatched)); // Default enabled
}

void TestCategoryFilter::testLongCategoryNames()
{
    QString longCategory = QString("very.long.category.name.with.many.components.").repeated(10);
    QString rules = longCategory + "=false";
    
    CategoryFilter filter(rules);
    
    auto msg = createMessage(longCategory);
    QVERIFY(!filter.filter(msg));
    
    auto msgSimilar = createMessage(longCategory + "extra");
    QVERIFY(filter.filter(msgSimilar)); // Should not match due to exact pattern
}

QTEST_MAIN(TestCategoryFilter)
#include "test_categoryfilter.moc"
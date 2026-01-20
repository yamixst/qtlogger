// Copyright (C) 2026 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QRegularExpression>

#include "qtlogger/sinks/rotatingfilesink.h"
#include "qtlogger/logmessage.h"

using namespace QtLogger;

class TestRotatingFileSink : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // Basic functionality tests
    void testCreateLogFile();
    void testWriteToLogFile();
    void testMultipleMessages();

    // Size-based rotation tests
    void testRotationBySize();
    void testRotationBySizeWithMultipleRotations();
    void testNoRotationWhenSizeNotExceeded();

    // Startup rotation tests
    void testRotationOnStartup();
    void testNoRotationOnStartupWhenEmpty();

    // Daily rotation tests
    void testRotationDaily();

    // File count limit tests
    void testMaxFileCountLimit();
    void testMaxFileCountOne();
    void testMaxFileCountZero();

    // Compression tests
    void testCompressionOption();

    // Rotated file naming tests
    void testRotatedFileNaming();
    void testRotatedFileNamingWithoutSuffix();

    // Combined options tests
    void testCombinedOptions();

    // Edge cases
    void testEmptyMessage();
    void testVeryLargeMessage();

private:
    LogMessage createLogMessage(const QString &message);
    LogMessage createLogMessageWithDate(const QString &message, const QDate &date);
    QStringList findRotatedFiles(const QString &basePath);
    int countFilesInDir(const QString &dirPath, const QString &pattern);

    QTemporaryDir *m_tempDir = nullptr;
};

void TestRotatingFileSink::init()
{
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
}

void TestRotatingFileSink::cleanup()
{
    delete m_tempDir;
    m_tempDir = nullptr;
}

LogMessage TestRotatingFileSink::createLogMessage(const QString &message)
{
    QMessageLogContext context("test.cpp", 42, "testFunction", "test.category");
    auto lmsg = LogMessage(QtDebugMsg, context, message);
    lmsg.setFormattedMessage(message);
    return lmsg;
}

LogMessage TestRotatingFileSink::createLogMessageWithDate(const QString &message, const QDate &date)
{
    QMessageLogContext context("test.cpp", 42, "testFunction", "test.category");
    auto lmsg = LogMessage(QtDebugMsg, context, message);
    lmsg.setFormattedMessage(message);
    return lmsg;
}

QStringList TestRotatingFileSink::findRotatedFiles(const QString &basePath)
{
    auto fi = QFileInfo(basePath);
    auto dir = QDir(fi.absolutePath());
    auto baseName = fi.completeBaseName();
    auto suffix = fi.suffix();

    QString pattern;
    if (suffix.isEmpty()) {
        pattern = QStringLiteral("^%1\\.\\d{4}-\\d{2}-\\d{2}\\.\\d+(\\.gz)?$")
                      .arg(QRegularExpression::escape(baseName));
    } else {
        pattern = QStringLiteral("^%1\\.\\d{4}-\\d{2}-\\d{2}\\.\\d+\\.%2(\\.gz)?$")
                      .arg(QRegularExpression::escape(baseName),
                           QRegularExpression::escape(suffix));
    }

    auto re = QRegularExpression(pattern);
    auto result = QStringList();

    const auto entries = dir.entryList(QDir::Files);
    for (const auto &entry : entries) {
        if (re.match(entry).hasMatch()) {
            result.append(dir.filePath(entry));
        }
    }

    return result;
}

int TestRotatingFileSink::countFilesInDir(const QString &dirPath, const QString &pattern)
{
    auto dir = QDir(dirPath);
    auto re = QRegularExpression(pattern);
    auto count = 0;

    const auto entries = dir.entryList(QDir::Files);
    for (const auto &entry : entries) {
        if (re.match(entry).hasMatch()) {
            ++count;
        }
    }

    return count;
}

void TestRotatingFileSink::testCreateLogFile()
{
    auto logPath = m_tempDir->filePath("test.log");
    auto sink = RotatingFileSink(logPath);

    auto lmsg = createLogMessage("Test message");
    sink.send(lmsg);

    QVERIFY(QFile::exists(logPath));
}

void TestRotatingFileSink::testWriteToLogFile()
{
    auto logPath = m_tempDir->filePath("test.log");
    auto sink = RotatingFileSink(logPath);

    auto lmsg = createLogMessage("Hello, World!");
    sink.send(lmsg);
    sink.flush();

    auto file = QFile(logPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    auto content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(content.contains("Hello, World!"));
}

void TestRotatingFileSink::testMultipleMessages()
{
    auto logPath = m_tempDir->filePath("test.log");
    auto sink = RotatingFileSink(logPath, 1024 * 1024, 5);

    for (int i = 0; i < 10; ++i) {
        auto lmsg = createLogMessage(QString("Message %1").arg(i));
        sink.send(lmsg);
    }
    sink.flush();

    auto file = QFile(logPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    auto content = QString::fromUtf8(file.readAll());
    file.close();

    for (int i = 0; i < 10; ++i) {
        QVERIFY(content.contains(QString("Message %1").arg(i)));
    }
}

void TestRotatingFileSink::testRotationBySize()
{
    auto logPath = m_tempDir->filePath("test.log");
    auto maxSize = 100; // Small size to trigger rotation quickly
    auto sink = RotatingFileSink(logPath, maxSize, 5);

    // Write messages until rotation occurs
    for (int i = 0; i < 20; ++i) {
        auto lmsg = createLogMessage(QString("Message number %1 with some extra text").arg(i));
        sink.send(lmsg);
    }
    sink.flush();

    auto rotatedFiles = findRotatedFiles(logPath);
    QVERIFY(rotatedFiles.size() >= 1);
}

void TestRotatingFileSink::testRotationBySizeWithMultipleRotations()
{
    auto logPath = m_tempDir->filePath("test.log");
    auto maxSize = 50;
    auto sink = RotatingFileSink(logPath, maxSize, 10);

    // Write many messages to cause multiple rotations
    for (int i = 0; i < 50; ++i) {
        auto lmsg = createLogMessage(QString("Long message number %1 with extra content").arg(i));
        sink.send(lmsg);
    }
    sink.flush();

    auto rotatedFiles = findRotatedFiles(logPath);
    QVERIFY(rotatedFiles.size() >= 2);
}

void TestRotatingFileSink::testNoRotationWhenSizeNotExceeded()
{
    auto logPath = m_tempDir->filePath("test.log");
    auto maxSize = 1024 * 1024; // 1MB - much larger than our messages
    auto sink = RotatingFileSink(logPath, maxSize, 5);

    for (int i = 0; i < 5; ++i) {
        auto lmsg = createLogMessage(QString("Short msg %1").arg(i));
        sink.send(lmsg);
    }
    sink.flush();

    auto rotatedFiles = findRotatedFiles(logPath);
    QCOMPARE(rotatedFiles.size(), 0);
}

void TestRotatingFileSink::testRotationOnStartup()
{
    auto logPath = m_tempDir->filePath("startup.log");

    // Create initial log file with content
    {
        auto file = QFile(logPath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("Previous log content\n");
        file.close();
    }

    // Create sink with RotationOnStartup option
    auto sink = RotatingFileSink(logPath, 1024 * 1024, 5, RotatingFileSink::RotationOnStartup);

    auto lmsg = createLogMessage("New message after startup");
    sink.send(lmsg);
    sink.flush();

    auto rotatedFiles = findRotatedFiles(logPath);
    QCOMPARE(rotatedFiles.size(), 1);

    // Verify new log file doesn't contain old content
    auto file = QFile(logPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    auto content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(!content.contains("Previous log content"));
    QVERIFY(content.contains("New message after startup"));
}

void TestRotatingFileSink::testNoRotationOnStartupWhenEmpty()
{
    auto logPath = m_tempDir->filePath("empty_startup.log");

    // Create empty log file
    {
        auto file = QFile(logPath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.close();
    }

    auto sink = RotatingFileSink(logPath, 1024 * 1024, 5, RotatingFileSink::RotationOnStartup);

    auto lmsg = createLogMessage("Message");
    sink.send(lmsg);
    sink.flush();

    auto rotatedFiles = findRotatedFiles(logPath);
    QCOMPARE(rotatedFiles.size(), 0);
}

void TestRotatingFileSink::testRotationDaily()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    auto logPath = m_tempDir->filePath("daily.log");

    // Create a log file and set its modification time to yesterday
    {
        auto file = QFile(logPath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("Yesterday's log content\n");
        file.close();

        // Set file modification time to yesterday
        auto yesterday = QDateTime::currentDateTime().addDays(-1);
        QVERIFY(file.open(QIODevice::ReadWrite));
        file.setFileTime(yesterday, QFileDevice::FileModificationTime);
        file.close();
    }

    auto sink = RotatingFileSink(logPath, 1024 * 1024, 5, RotatingFileSink::RotationDaily);

    auto lmsg = createLogMessage("Today's message");
    sink.send(lmsg);
    sink.flush();

    auto rotatedFiles = findRotatedFiles(logPath);
    QCOMPARE(rotatedFiles.size(), 1);
#else
    QSKIP("QFile::setFileTime requires Qt 5.10 or later");
#endif
}

void TestRotatingFileSink::testMaxFileCountLimit()
{
    auto logPath = m_tempDir->filePath("limited.log");
    auto maxSize = 30;
    auto maxFileCount = 3;
    auto sink = RotatingFileSink(logPath, maxSize, maxFileCount);

    // Write many messages to exceed max file count
    for (int i = 0; i < 100; ++i) {
        auto lmsg = createLogMessage(QString("Message %1 with extra text here").arg(i));
        sink.send(lmsg);
    }
    sink.flush();

    auto rotatedFiles = findRotatedFiles(logPath);
    // maxFileCount - 1 rotated files (main file is not counted)
    QVERIFY(rotatedFiles.size() <= maxFileCount - 1);
}

void TestRotatingFileSink::testMaxFileCountOne()
{
    auto logPath = m_tempDir->filePath("single.log");
    auto maxSize = 50;
    auto maxFileCount = 1; // Only main file, no rotation
    auto sink = RotatingFileSink(logPath, maxSize, maxFileCount);

    for (int i = 0; i < 20; ++i) {
        auto lmsg = createLogMessage(QString("Message %1 with some content").arg(i));
        sink.send(lmsg);
    }
    sink.flush();

    auto rotatedFiles = findRotatedFiles(logPath);
    QCOMPARE(rotatedFiles.size(), 0);
}

void TestRotatingFileSink::testMaxFileCountZero()
{
    auto logPath = m_tempDir->filePath("unlimited.log");
    auto maxSize = 30;
    auto maxFileCount = 0; // Unlimited rotated files
    auto sink = RotatingFileSink(logPath, maxSize, maxFileCount);

    for (int i = 0; i < 30; ++i) {
        auto lmsg = createLogMessage(QString("Message %1 with content").arg(i));
        sink.send(lmsg);
    }
    sink.flush();

    auto rotatedFiles = findRotatedFiles(logPath);
    // Should have many rotated files since no limit
    QVERIFY(rotatedFiles.size() >= 3);
}

void TestRotatingFileSink::testCompressionOption()
{
    auto logPath = m_tempDir->filePath("compressed.log");
    auto maxSize = 50;
    auto sink = RotatingFileSink(logPath, maxSize, 5, RotatingFileSink::Compression);

    for (int i = 0; i < 30; ++i) {
        auto lmsg = createLogMessage(QString("Message %1 with extra content").arg(i));
        sink.send(lmsg);
    }
    sink.flush();

    auto dir = QDir(m_tempDir->path());
    auto entries = dir.entryList(QStringList() << "*.gz", QDir::Files);
    QVERIFY(entries.size() >= 1);
}

void TestRotatingFileSink::testRotatedFileNaming()
{
    auto logPath = m_tempDir->filePath("named.log");
    auto maxSize = 50;
    auto sink = RotatingFileSink(logPath, maxSize, 5);

    for (int i = 0; i < 20; ++i) {
        auto lmsg = createLogMessage(QString("Message %1 with content").arg(i));
        sink.send(lmsg);
    }
    sink.flush();

    auto rotatedFiles = findRotatedFiles(logPath);
    QVERIFY(rotatedFiles.size() >= 1);

    // Verify naming format: named.YYYY-MM-DD.N.log
    auto dateStr = QDate::currentDate().toString("yyyy-MM-dd");
    auto re = QRegularExpression(QString("named\\.%1\\.\\d+\\.log").arg(QRegularExpression::escape(dateStr)));

    auto foundMatch = false;
    for (const auto &filePath : rotatedFiles) {
        auto fileName = QFileInfo(filePath).fileName();
        if (re.match(fileName).hasMatch()) {
            foundMatch = true;
            break;
        }
    }
    QVERIFY(foundMatch);
}

void TestRotatingFileSink::testRotatedFileNamingWithoutSuffix()
{
    auto logPath = m_tempDir->filePath("nosuffix");
    auto maxSize = 50;
    auto sink = RotatingFileSink(logPath, maxSize, 5);

    for (int i = 0; i < 20; ++i) {
        auto lmsg = createLogMessage(QString("Message %1 with content").arg(i));
        sink.send(lmsg);
    }
    sink.flush();

    auto rotatedFiles = findRotatedFiles(logPath);
    QVERIFY(rotatedFiles.size() >= 1);

    // Verify naming format without suffix: nosuffix.YYYY-MM-DD.N
    auto dateStr = QDate::currentDate().toString("yyyy-MM-dd");
    auto re = QRegularExpression(QString("nosuffix\\.%1\\.\\d+$").arg(QRegularExpression::escape(dateStr)));

    auto foundMatch = false;
    for (const auto &filePath : rotatedFiles) {
        auto fileName = QFileInfo(filePath).fileName();
        if (re.match(fileName).hasMatch()) {
            foundMatch = true;
            break;
        }
    }
    QVERIFY(foundMatch);
}

void TestRotatingFileSink::testCombinedOptions()
{
    auto logPath = m_tempDir->filePath("combined.log");

    // Create existing file to trigger startup rotation
    {
        auto file = QFile(logPath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("Existing content\n");
        file.close();
    }

    auto options = RotatingFileSink::RotationOnStartup | RotatingFileSink::Compression;
    auto sink = RotatingFileSink(logPath, 50, 5, options);

    for (int i = 0; i < 20; ++i) {
        auto lmsg = createLogMessage(QString("Message %1 content here").arg(i));
        sink.send(lmsg);
    }
    sink.flush();

    auto dir = QDir(m_tempDir->path());
    auto gzFiles = dir.entryList(QStringList() << "*.gz", QDir::Files);
    QVERIFY(gzFiles.size() >= 1);
}

void TestRotatingFileSink::testEmptyMessage()
{
    auto logPath = m_tempDir->filePath("empty_msg.log");
    auto sink = RotatingFileSink(logPath);

    auto lmsg = createLogMessage("");
    sink.send(lmsg);
    sink.flush();

    QVERIFY(QFile::exists(logPath));
}

void TestRotatingFileSink::testVeryLargeMessage()
{
    auto logPath = m_tempDir->filePath("large_msg.log");
    auto sink = RotatingFileSink(logPath, 1024 * 1024, 5);

    auto largeContent = QString(10000, 'X');
    auto lmsg = createLogMessage(largeContent);
    sink.send(lmsg);
    sink.flush();

    auto file = QFile(logPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    auto content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(content.contains(largeContent));
}

QTEST_MAIN(TestRotatingFileSink)
#include "test_rotatingfilesink.moc"
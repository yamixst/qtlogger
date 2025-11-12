// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "rotatingfilesink.h"

/*
 * Rotated file naming format:
 *   <basename>.<date>.<index>.<suffix>[.gz]
 *   Example: app.2024-05-15.1.log or app.2024-05-15.1.log.gz
 *
 * Rotation triggers:
 *   1. On startup (RotationOnStartup) - rotates if the log file is non-empty
 *   2. Daily (RotationDaily) - rotates when the message date differs from the current log date
 *   3. By size (maxFileSize > 0) - rotates when adding a new message would exceed maxFileSize
 *
 *   If maxFileCount == 1, rotation is disabled (only the main file is kept)
 *   If maxFileCount <= 0, rotated files are kept indefinitely (no automatic cleanup)
 */


#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QtEndian>
#include <QDataStream>

#include <algorithm>
#include <iostream>

namespace QtLogger {

namespace {

QTLOGGER_DECL_SPEC
static quint32 calculateCRC32(QFile &file) {
    quint32 crc = 0xFFFFFFFF;
    const quint32 polynomial = 0xEDB88320;
    static quint32 table[256];
    static bool tableGenerated = false;

    if (!tableGenerated) {
        for (quint32 i = 0; i < 256; i++) {
            quint32 value = i;
            for (int j = 0; j < 8; j++) {
                if (value & 1) value = (value >> 1) ^ polynomial;
                else value >>= 1;
            }
            table[i] = value;
        }
        tableGenerated = true;
    }

    file.seek(0);
    char buffer[8192];
    while (!file.atEnd()) {
        auto bytesRead = file.read(buffer, sizeof(buffer));
        for (auto i = 0; i < bytesRead; i++) {
            crc = table[(crc ^ static_cast<unsigned char>(buffer[i])) & 0xFF] ^ (crc >> 8);
        }
    }
    return crc ^ 0xFFFFFFFF;
}

} // namespace

class RotatingFileSink::RotatingFileSinkPrivate
{
public:
    RotatingFileSinkPrivate(RotatingFileSink *q,
                            int maxFileSize,
                            int maxFileCount,
                            RotatingFileSink::Options options)
        : q_ptr(q)
        , m_maxFileSize(maxFileSize)
        , m_maxFileCount(maxFileCount)
        , m_rotationOnStartup(options.testFlag(RotatingFileSink::RotationOnStartup))
        , m_rotationDaily(options.testFlag(RotatingFileSink::RotationDaily))
        , m_compression(options.testFlag(RotatingFileSink::Compression))
    {
    }

    void init()
    {
        if (m_initialized)
            return;

        m_initialized = true;

        auto fi = QFileInfo(q_ptr->file()->fileName());
        if (fi.exists() && fi.size() > 0) {
            m_currentLogDate = fi.lastModified().date();
        } else {
            m_currentLogDate = QDate::currentDate();
        }

        if (m_rotationOnStartup) {
            checkStartupRotation();
        }
    }

    void rotateIfNeeded(const LogMessage &lmsg)
    {
        const auto messageDate = lmsg.time().date();

        if (m_rotationDaily) {
            checkDailyRotation(messageDate);
        }

        if (m_maxFileSize > 0) {
            const auto additionalSize = lmsg.formattedMessage().toUtf8().size() + 1; // +1 for newline
            checkSizeRotation(additionalSize);
        }
    }

    void checkStartupRotation()
    {
        if (q_ptr->file()->size() > 0) {
            rotate();
        }
    }

    void checkDailyRotation(const QDate &messageDate)
    {
        if (messageDate != m_currentLogDate && q_ptr->file()->size() > 0) {
            rotate();
            m_currentLogDate = messageDate;
        }
    }

    void checkSizeRotation(int additionalSize)
    {
        if (m_maxFileSize <= 0)
            return;

        const auto currentSize = q_ptr->file()->size();
        if (currentSize > 0 && (currentSize + additionalSize) > m_maxFileSize) {
            rotate();
        }
    }

    QString baseDir() const
    {
        auto fi = QFileInfo(q_ptr->file()->fileName());
        return fi.absolutePath();
    }

    QString generateRotatedFileName(const QDate &date, int index) const
    {
        auto fi = QFileInfo(q_ptr->file()->fileName());
        const auto baseName = fi.completeBaseName();
        const auto suffix = fi.suffix();
        const auto dateStr = date.toString(QStringLiteral("yyyy-MM-dd"));

        QString rotatedName;
        if (suffix.isEmpty()) {
            rotatedName = QStringLiteral("%1.%2.%3").arg(baseName, dateStr).arg(index);
        } else {
            rotatedName = QStringLiteral("%1.%2.%3.%4").arg(baseName, dateStr).arg(index).arg(suffix);
        }

        return QDir(baseDir()).filePath(rotatedName);
    }

    int findNextIndexForDate(const QDate &date) const
    {
        auto fi = QFileInfo(q_ptr->file()->fileName());
        const auto baseName = fi.completeBaseName();
        const auto suffix = fi.suffix();
        const auto dateStr = date.toString(QStringLiteral("yyyy-MM-dd"));

        QString pattern;
        if (suffix.isEmpty()) {
            pattern = QStringLiteral("^%1\\.%2\\.(\\d+)(\\.gz)?$")
                          .arg(QRegularExpression::escape(baseName),
                               QRegularExpression::escape(dateStr));
        } else {
            pattern = QStringLiteral("^%1\\.%2\\.(\\d+)\\.%3(\\.gz)?$")
                          .arg(QRegularExpression::escape(baseName),
                               QRegularExpression::escape(dateStr),
                               QRegularExpression::escape(suffix));
        }

        auto re = QRegularExpression(pattern);
        auto dir = QDir(baseDir());
        auto maxIndex = 0;

        const auto entries = dir.entryList(QDir::Files);
        for (const QString &entry : entries) {
            auto match = re.match(entry);
            if (match.hasMatch()) {
                auto index = match.captured(1).toInt();
                if (index > maxIndex) {
                    maxIndex = index;
                }
            }
        }

        return maxIndex + 1;
    }

    void compressFile(const QString &filePath)
    {
        auto inputFile = QFile(filePath);
        if (!inputFile.open(QIODevice::ReadOnly)) return;

        const auto compressedPath = filePath + QStringLiteral(".gz");
        auto outputFile = QFile(compressedPath);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            inputFile.close();
            return;
        }

        auto fileCRC = calculateCRC32(inputFile);
        auto fileSize = static_cast<quint32>(inputFile.size());
        inputFile.seek(0);

        outputFile.putChar('\x1f'); // ID1
        outputFile.putChar('\x8b'); // ID2
        outputFile.putChar('\x08'); // CM (Deflate)
        outputFile.putChar('\x00'); // FLG
        outputFile.write("\x00\x00\x00\x00", 4); // MTIME (0 = unknown)
        outputFile.putChar('\x00'); // XFL
        outputFile.putChar('\x03'); // OS (Unix)

        auto rawData = inputFile.readAll();
        auto compressed = qCompress(rawData, 5);

        if (compressed.size() > 10) {
            outputFile.write(compressed.constData() + 6, compressed.size() - 6 - 4);
        }

        auto le_crc = qToLittleEndian(fileCRC);
        auto le_size = qToLittleEndian(fileSize);
        outputFile.write(reinterpret_cast<const char*>(&le_crc), 4);
        outputFile.write(reinterpret_cast<const char*>(&le_size), 4);

        inputFile.close();
        outputFile.close();
        QFile::remove(filePath);
    }

    QStringList findRotatedFiles() const
    {
        auto fi = QFileInfo(q_ptr->file()->fileName());
        const auto baseName = fi.completeBaseName();
        const auto suffix = fi.suffix();

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
        auto dir = QDir(baseDir());
        auto result = QStringList();

        const auto entries = dir.entryList(QDir::Files, QDir::Name);
        for (const QString &entry : entries) {
            if (re.match(entry).hasMatch()) {
                result.append(dir.filePath(entry));
            }
        }

        std::sort(result.begin(), result.end(), [](const QString &a, const QString &b) {
            return QFileInfo(a).lastModified() < QFileInfo(b).lastModified();
        });

        return result;
    }

    void removeOldFiles()
    {
        if (m_maxFileCount <= 0)
            return;

        auto rotatedFiles = findRotatedFiles();

        while (rotatedFiles.size() > m_maxFileCount - 1) {
            const QString &oldestFile = rotatedFiles.first();
            if (!QFile::remove(oldestFile)) {
                std::cerr << "RotatingFileSink: Failed to remove old log file: "
                          << oldestFile.toStdString() << std::endl;
            }
            rotatedFiles.removeFirst();
        }
    }

    void rotate()
    {
        if (m_maxFileCount == 1)
            return;

        q_ptr->file()->close();

        const auto &currentFileName = q_ptr->file()->fileName();
        const auto rotationDate = m_currentLogDate.isValid() ? m_currentLogDate : QDate::currentDate();
        const auto nextIndex = findNextIndexForDate(rotationDate);
        const auto rotatedFileName = generateRotatedFileName(rotationDate, nextIndex);

        if (!QFile::rename(currentFileName, rotatedFileName)) {
            std::cerr << "RotatingFileSink: Failed to rename log file from "
                      << currentFileName.toStdString() << " to "
                      << rotatedFileName.toStdString() << std::endl;
        } else if (m_compression) {
            compressFile(rotatedFileName);
        }

        removeOldFiles();

        if (!q_ptr->file()->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            std::cerr << "RotatingFileSink: Failed to reopen log file: "
                      << currentFileName.toStdString() << std::endl;
        }

        m_currentLogDate = QDate::currentDate();
    }

    RotatingFileSink *q_ptr;

    int m_maxFileSize;
    int m_maxFileCount;
    bool m_rotationOnStartup;
    bool m_rotationDaily;
    bool m_compression;

    QDate m_currentLogDate;
    bool m_initialized = false;
};

QTLOGGER_DECL_SPEC
RotatingFileSink::RotatingFileSink(const QString &path,
                                   int maxFileSize,
                                   int maxFileCount,
                                   RotatingFileSink::Options options)
    : FileSink(path)
    , d(new RotatingFileSinkPrivate(this, maxFileSize, maxFileCount, options))
{
}

QTLOGGER_DECL_SPEC
RotatingFileSink::~RotatingFileSink() = default;

QTLOGGER_DECL_SPEC
void RotatingFileSink::send(const LogMessage &lmsg)
{
    d->init();
    d->rotateIfNeeded(lmsg);
    FileSink::send(lmsg);
}

} // namespace QtLogger

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "rotatingfilesink.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

namespace QtLogger {

QTLOGGER_DECL_SPEC
RotatingFileSink::RotatingFileSink(const QString &path, int maxFileSize, int maxFileCount)
    : FileSink(path), m_maxFileSize(maxFileSize), m_maxFileCount(maxFileCount)
{
    if (file()->size() > 0)
        rotate();
}

QTLOGGER_DECL_SPEC
void RotatingFileSink::send(const LogMessage &lmsg)
{
    const auto newFileSize = file()->size() + lmsg.formattedMessage().toLocal8Bit().size();

    if (m_maxFileSize > 0 && file()->size() != 0 && newFileSize > m_maxFileSize) {
        rotate();
    }

    FileSink::send(lmsg);
}

QTLOGGER_DECL_SPEC
QString numberedFileName(const QString &fileName, int i)
{
    return QStringLiteral("%1.%2").arg(fileName).arg(i);
}

QTLOGGER_DECL_SPEC
void RotatingFileSink::rotate()
{
    if (m_maxFileCount == 1)
        return;

    int maxFileCount = m_maxFileCount > 1 ? m_maxFileCount : RotatingFileCountLimit;

    file()->close();

    const auto &fileName = file()->fileName();

    int i = 1;
    for (; i < maxFileCount; ++i) {
        if (!QFile::exists(numberedFileName(fileName, i))) {
            --i;
            break;
        }
    }
    for (; i > 0; --i) {
        const auto &curFileName = numberedFileName(fileName, i);
        const auto &newFileName = numberedFileName(fileName, i + 1);

        QFile::rename(curFileName, newFileName);
    }

    // rename first
    QFile::rename(fileName, numberedFileName(fileName, 1));

    // remove last
    QFile::remove(QStringLiteral("%1.%2").arg(fileName).arg(m_maxFileCount));

    file()->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
}

} // namespace QtLogger

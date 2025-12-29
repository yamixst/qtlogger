// Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include "filesink.h"

#include <QDateTime>
#include <QFile>
#include <QRegularExpression>
#include <QSharedPointer>

#include <iostream>

namespace QtLogger {

namespace {

/**
 * @brief Replaces the time pattern in the given string with the current date and time.
 *
 * This function searches for a time pattern in the format `%{time <format>}` within the input
 * string. If found, it replaces the pattern with the current date and time formatted according to
 * the specified format. If no format is specified, it defaults to "yyyyMMdd_hhmmss".
 *
 * @param path The input string potentially containing the time pattern.
 * @return A new string with the time pattern replaced by the current date and time.
 */

QTLOGGER_DECL_SPEC
QString replaceTimePattern(const QString &path)
{
    static auto re = QRegularExpression(QStringLiteral("(.*)%{time *(.*?)}(.*)"));
    auto match = re.match(path);

    if (!match.hasMatch())
        return path;

    auto format = match.captured(2);

    if (format.isEmpty()) {
        format = QStringLiteral("yyyyMMdd_hhmmss");
    }

    return match.captured(1) + QDateTime::currentDateTime().toString(format) + match.captured(3);
}

QTLOGGER_DECL_SPEC
QSharedPointer<QFile> createFilePtr(const QString &path)
{
    return QSharedPointer<QFile>::create(replaceTimePattern(path));
}

}

QTLOGGER_DECL_SPEC
FileSink::FileSink(const QString &path) : IODeviceSink(createFilePtr(path))
{
    if (!file()->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        std::cerr << "FileSink: Can't open log file: " << path.toStdString()
                  << " error: " << file()->errorString().toStdString() << std::endl;
    }
}

QTLOGGER_DECL_SPEC
FileSink::~FileSink()
{
    file()->close();
}

QTLOGGER_DECL_SPEC
bool FileSink::flush()
{
    return file()->flush();
}

QTLOGGER_DECL_SPEC
QFile *FileSink::file() const
{
    return qobject_cast<QFile *>(device().data());
}

} // namespace QtLogger

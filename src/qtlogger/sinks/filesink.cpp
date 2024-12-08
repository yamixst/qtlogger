// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include "filesink.h"

#include <QDateTime>
#include <QFile>
#include <QRegularExpression>
#include <QSharedPointer>

#include <iostream>

namespace QtLogger {

namespace {

QTLOGGER_DECL_SPEC
QString replaceTimePattern(const QString &str)
{
    static auto re = QRegularExpression(QStringLiteral("(.*)%{time *(.*?)}(.*)"));
    auto match = re.match(str);

    if (!match.hasMatch())
        return str;

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
        std::cerr << "Logger::FileHandler: Can't open log file: " << path.toStdString()
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

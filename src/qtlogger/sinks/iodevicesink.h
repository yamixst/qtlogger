// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

#include <QIODevice>
#include <QSharedPointer>

#include "../logger_global.h"
#include "../sink.h"

namespace QtLogger {

using QIODevicePtr = QSharedPointer<QIODevice>;

class QTLOGGER_EXPORT IODeviceSink : public Sink
{
public:
    explicit IODeviceSink(const QIODevicePtr &device);

    void send(const LogMessage &lmsg) override;

protected:
    const QIODevicePtr &device() const;
    void setDevice(const QIODevicePtr &device);

private:
    QIODevicePtr m_device;
};

using IODeviceSinkPtr = QSharedPointer<IODeviceSink>;

} // namespace QtLogger

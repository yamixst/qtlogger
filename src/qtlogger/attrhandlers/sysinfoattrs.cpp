// Copyright (C) 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include "sysinfoattrs.h"

#include <QSysInfo>

namespace QtLogger {

QTLOGGER_DECL_SPEC
SysInfoAttrs::SysInfoAttrs()
{
    m_attrs = QVariantHash {
        { QStringLiteral("os_name"), QSysInfo::productType() },
        { QStringLiteral("os_version"), QSysInfo::productVersion() },
        { QStringLiteral("kernel_type"), QSysInfo::kernelType() },
        { QStringLiteral("kernel_version"), QSysInfo::kernelVersion() },
        { QStringLiteral("cpu_arch"), QSysInfo::currentCpuArchitecture() },
        { QStringLiteral("build_abi"), QSysInfo::buildAbi() },
        { QStringLiteral("build_cpu_arch"), QSysInfo::buildCpuArchitecture() },
        { QStringLiteral("pretty_product_name"), QSysInfo::prettyProductName() },
        { QStringLiteral("machine_host_name"), QSysInfo::machineHostName() },
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        { QStringLiteral("machine_unique_id"), QString::fromLatin1(QSysInfo::machineUniqueId()) },
        { QStringLiteral("boot_unique_id"), QString::fromLatin1(QSysInfo::bootUniqueId()) },
#endif
    };
}

QTLOGGER_DECL_SPEC
QVariantHash SysInfoAttrs::attributes(const LogMessage &lmsg)
{
    Q_UNUSED(lmsg)
    return m_attrs;
}

} // namespace QtLogger

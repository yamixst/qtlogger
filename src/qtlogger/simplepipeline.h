#pragma once

#include <QList>
#include <QSharedPointer>

#include "logger_global.h"
#include "typedpipeline.h"

namespace QtLogger {

class QTLOGGER_EXPORT SimplePipeline : public TypedPipeline
{
public:
    explicit SimplePipeline(bool scoped = false, SimplePipeline *parent = nullptr)
        : TypedPipeline(scoped), m_parent(parent)
    {
    }

    SimplePipeline &addSeqNumber();
    SimplePipeline &addAppInfo();
#ifdef QTLOGGER_NETWORK
    SimplePipeline &addHostInfo();
#endif

    SimplePipeline &filter(std::function<bool(const LogMessage &)> func);
    SimplePipeline &filter(const QString &regexp);
    SimplePipeline &filterCategory(const QString &rules);
    SimplePipeline &filterDuplicate();

    SimplePipeline &format(std::function<QString(const LogMessage &)> func);
    SimplePipeline &format(const QString &pattern);
    SimplePipeline &formatByQt();
    SimplePipeline &formatPretty();
    SimplePipeline &formatToJson();

    SimplePipeline &sendToStdOut();
    SimplePipeline &sendToStdErr();
#ifdef QTLOGGER_SYSLOG
    SimplePipeline &sendToSyslog();
#endif
#ifdef QTLOGGER_SDJOURNAL
    SimplePipeline &sendToSdJournal();
#endif
    SimplePipeline &sendToPlatformStdLog();
    SimplePipeline &sendToFile(const QString &fileName, int maxFileSize = 0, int maxFileCount = 0);
#ifdef QTLOGGER_NETWORK
    SimplePipeline &sendToHttp(const QString &url);
#endif

    SimplePipeline &pipeline();
    SimplePipeline &end();

private:
    SimplePipeline *m_parent = nullptr;
};

using SimplePipelinePtr = QSharedPointer<SimplePipeline>;

} // namespace QtLogger

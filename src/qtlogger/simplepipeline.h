#pragma once

#include <QList>
#include <QSharedPointer>

#include "logger_global.h"
#include "sortedpipeline.h"

namespace QtLogger {

class QTLOGGER_EXPORT SimplePipeline : public SortedPipeline
{
public:
    explicit SimplePipeline(bool scoped = false, SimplePipeline *parent = nullptr)
        : SortedPipeline(scoped), m_parent(parent)
    {
    }

    SimplePipeline &addSeqNumber(const QString &name = QStringLiteral("seq_number"));
    SimplePipeline &addAppInfo();
#ifdef QTLOGGER_NETWORK
    SimplePipeline &addHostInfo();
#endif
    SimplePipeline &attrHandler(std::function<QVariantHash(const LogMessage &lmsg)> func);

    SimplePipeline &filter(std::function<bool(const LogMessage &)> func);
    SimplePipeline &filter(const QString &regexp);
    SimplePipeline &filterCategory(const QString &rules);
    SimplePipeline &filterDuplicate();
    SimplePipeline &filterLevel(QtMsgType minLevel);

    SimplePipeline &format(std::function<QString(const LogMessage &)> func);
    SimplePipeline &format(const QString &pattern);
    SimplePipeline &formatByQt();
    SimplePipeline &formatPretty();
    SimplePipeline &formatToJson();

    SimplePipeline &sendToStdOut(bool colorEnabled = false);
    SimplePipeline &sendToStdErr(bool colorEnabled = false);
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
#ifdef Q_OS_WIN
    SimplePipeline &sendToWinDebug();
#endif

    SimplePipeline &pipeline();
    SimplePipeline &end();

    SimplePipeline &handler(std::function<bool(LogMessage &)> func);

    virtual void flush();

private:
    static void recursiveFlush(const Pipeline *pipeline);

    SimplePipeline *m_parent = nullptr;
};

using SimplePipelinePtr = QSharedPointer<SimplePipeline>;

} // namespace QtLogger

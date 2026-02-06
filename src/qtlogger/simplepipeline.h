#pragma once

#include <QList>
#include <QSharedPointer>

#include "logger_global.h"
#include "sortedpipeline.h"
#include "sinks/iodevicesink.h"
#include "sinks/rotatingfilesink.h"

QT_FORWARD_DECLARE_CLASS(QIODevice)

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
    SimplePipeline &addSysInfo();
#ifdef QTLOGGER_NETWORK
    SimplePipeline &addHostInfo();
#endif
    SimplePipeline &attrHandler(std::function<QVariantHash(const LogMessage &lmsg)> func);

    SimplePipeline &filter(std::function<bool(const LogMessage &)> func);
    SimplePipeline &filter(const QString &regexp);
    SimplePipeline &filterLevel(QtMsgType minLevel);
    SimplePipeline &filterCategory(const QString &rules);
    SimplePipeline &filterDuplicate();

    SimplePipeline &format(std::function<QString(const LogMessage &)> func);
    SimplePipeline &format(const QString &pattern);
    SimplePipeline &formatByQt();
    SimplePipeline &formatPretty(bool colorize = false, int maxCategoryWidth = 15);
    SimplePipeline &formatToJson(bool compact = false);
    SimplePipeline &formatToSentry(const QString &sdkName = QStringLiteral("qtlogger.sentry"),
                                   const QString &sdkVersion = QStringLiteral("1.0.0"));

    SimplePipeline &sendToStdOut(bool colorize = false);
    SimplePipeline &sendToStdErr(bool colorize = false);
#ifdef QTLOGGER_SYSLOG
    SimplePipeline &sendToSyslog();
#endif
#ifdef QTLOGGER_SDJOURNAL
    SimplePipeline &sendToSdJournal();
#endif
    SimplePipeline &sendToPlatformStdLog();
    SimplePipeline &sendToFile(const QString &fileName, int maxFileSize = 0, int maxFileCount = 0, RotatingFileSink::Options options = RotatingFileSink::None);
    SimplePipeline &sendToIODevice(const QIODevicePtr &device);
    SimplePipeline &sendToSignal(QObject *receiver, const char *method);
#ifdef QTLOGGER_NETWORK
    SimplePipeline &sendToHttp(const QString &url);
    SimplePipeline &sendToHttp(const QString &url,
                               const QList<QPair<QByteArray, QByteArray>> &headers);
#endif
#ifdef Q_OS_WIN
    SimplePipeline &sendToWinDebug();
#endif
#ifdef QTLOGGER_ANDROIDLOG
    SimplePipeline &sendToAndroidLog();
#endif
#ifdef QTLOGGER_OSLOG
    SimplePipeline &sendToOsLog();
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

#include "simplepipeline.h"

#include <QCoreApplication>

#include "attrhandlers/appinfoattrs.h"
#include "attrhandlers/appuuidattr.h"
#include "attrhandlers/functionattrhandler.h"
#include "attrhandlers/seqnumberattr.h"
#include "attrhandlers/sysinfoattrs.h"
#include "filters/categoryfilter.h"
#include "filters/duplicatefilter.h"
#include "filters/functionfilter.h"
#include "filters/levelfilter.h"
#include "filters/regexpfilter.h"
#include "formatters/functionformatter.h"
#include "formatters/jsonformatter.h"
#include "formatters/patternformatter.h"
#include "formatters/prettyformatter.h"
#include "formatters/qtlogmessageformatter.h"
#include "formatters/sentryformatter.h"
#include "functionhandler.h"
#include "messagepatterns.h"
#include "sinks/platformstdsink.h"
#include "sinks/rotatingfilesink.h"
#include "sinks/stderrsink.h"
#include "sinks/stdoutsink.h"
#include "sinks/signalsink.h"

#ifdef QTLOGGER_NETWORK
#    include "attrhandlers/hostinfoattrs.h"
#    include "sinks/httpsink.h"
#endif

#ifdef QTLOGGER_SYSLOG
#    include "sinks/syslogsink.h"
#endif

#ifdef QTLOGGER_ANDROIDLOG
#    include "sinks/androidlogsink.h"
#endif

#ifdef QTLOGGER_OSLOG
#    include "sinks/oslogsink.h"
#endif

#ifdef QTLOGGER_SDJOURNAL
#    include "sinks/sdjournalsink.h"
#endif

#ifdef Q_OS_WIN
#    include "sinks/windebugsink.h"
#endif

namespace QtLogger {

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::addSeqNumber(const QString &name)
{
    append(SeqNumberAttrPtr::create(name));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::addAppInfo()
{
    append(AppInfoAttrsPtr::create());
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::addAppUuid(const QString &name)
{
    append(AppUuidAttrPtr::create(name));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::addSysInfo()
{
    append(SysInfoAttrsPtr::create());
    return *this;
}

#ifdef QTLOGGER_NETWORK
QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::addHostInfo()
{
    append(HostInfoAttrsPtr::create());
    return *this;
}
#endif

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::attrHandler(std::function<QVariantHash(const LogMessage &lmsg)> func)
{
    append(FunctionAttrHandlerPtr::create(func));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::filter(std::function<bool(const LogMessage &)> func)
{
    append(FunctionFilterPtr::create(func));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::filter(const QString &regexp)
{
    append(RegExpFilterPtr::create(regexp));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::filterLevel(QtMsgType minLevel)
{
    append(LevelFilterPtr::create(minLevel));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::filterCategory(const QString &rules)
{
    append(CategoryFilterPtr::create(rules));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::filterDuplicate()
{
    append(DuplicateFilterPtr::create());
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::format(std::function<QString(const LogMessage &)> func)
{
    append(FunctionFormatterPtr::create(func));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::format(const QString &pattern)
{
    if (pattern == "default")
        append(PatternFormatterPtr::create(DefaultMessagePattern));
    else if (pattern == "qt")
        append(QtLogMessageFormatter::instance());
    else if (pattern == "pretty")
        append(PrettyFormatterPtr::create());
    else
        append(PatternFormatterPtr::create(pattern));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::formatByQt()
{
    append(QtLogMessageFormatter::instance());
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::formatPretty(bool colorize, int maxCategoryWidth)
{
    append(PrettyFormatterPtr::create(colorize, maxCategoryWidth));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::formatToJson(bool compact)
{
    append(JsonFormatterPtr::create(compact));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::formatToSentry(const QString &sdkName, const QString &sdkVersion)
{
    append(SentryFormatterPtr::create(sdkName, sdkVersion));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToStdOut(bool colorize)
{
    append(StdOutSinkPtr::create(colorize ? ColorMode::Auto : ColorMode::Never));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToStdErr(bool colorize)
{
    append(StdErrSinkPtr::create(colorize ? ColorMode::Auto : ColorMode::Never));
    return *this;
}

#ifdef QTLOGGER_SYSLOG
QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToSyslog()
{
    append(SyslogSinkPtr::create(QCoreApplication::applicationName()));
    return *this;
}
#endif

#ifdef QTLOGGER_SDJOURNAL
QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToSdJournal()
{
    append(SdJournalSinkPtr::create());
    return *this;
}
#endif

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToPlatformStdLog()
{
    append(PlatformStdSinkPtr::create());
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToFile(const QString &fileName, int maxFileSize,
                                           int maxFileCount, RotatingFileSink::Options options)
{
    if (fileName.isEmpty())
        return *this;

    if (maxFileSize > 0
        || options.testFlag(RotatingFileSink::RotationOnStartup)
        || options.testFlag(RotatingFileSink::RotationDaily)) {
        append(RotatingFileSinkPtr::create(fileName, maxFileSize, maxFileCount, options));
    }
    else {
        append(FileSinkPtr::create(fileName));
    }

    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToIODevice(const QIODevicePtr &device)
{
    append(IODeviceSinkPtr::create(device));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToSignal(QObject *receiver, const char *method)
{
    auto sink = SignalSinkPtr::create();
    QObject::connect(sink.data(), SIGNAL(message(QtLogger::LogMessage)), receiver, method);
    append(sink.staticCast<Sink>());
    return *this;
}

#ifdef QTLOGGER_NETWORK
QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToHttp(const QString &url)
{
    append(HttpSinkPtr::create(QUrl(url)));
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToHttp(const QString &url,
                                           const QList<QPair<QByteArray, QByteArray>> &headers)
{
    append(HttpSinkPtr::create(QUrl(url), headers));
    return *this;
}
#endif

#ifdef Q_OS_WIN
QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToWinDebug()
{
    append(WinDebugSinkPtr::create());
    return *this;
}
#endif

#ifdef QTLOGGER_ANDROIDLOG
QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToAndroidLog()
{
    append(AndroidLogSinkPtr::create());
    return *this;
}
#endif

#ifdef QTLOGGER_OSLOG
QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToOsLog()
{
    append(OslogSinkPtr::create());
    return *this;
}
#endif

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::pipeline()
{
    auto pipeline = SimplePipelinePtr::create(/* scoped */ true, /* parent */ this);
    append(pipeline);
    return *pipeline.data();
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::end()
{
    if (m_parent)
        return *m_parent;
    else
        return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::handler(std::function<bool(LogMessage &)> func)
{
    append(FunctionHandlerPtr::create(std::move(func)));
    return *this;
}

QTLOGGER_DECL_SPEC
void SimplePipeline::recursiveFlush(const Pipeline *pipeline)
{
    for (const auto &handler : pipeline->handlers()) {
        if (auto sink = handler.dynamicCast<Sink>()) {
            sink->flush();
            continue;
        }
        if (auto pipeline = handler.dynamicCast<Pipeline>()) {
            recursiveFlush(pipeline.data());
        }
    }
}

QTLOGGER_DECL_SPEC
void SimplePipeline::flush()
{
    recursiveFlush(this);
}

} // namespace QtLogger

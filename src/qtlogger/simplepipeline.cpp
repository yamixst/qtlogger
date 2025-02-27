#include "simplepipeline.h"

#include <QCoreApplication>

#include "attrhandlers/appinfoattrs.h"
#include "attrhandlers/functionattr.h"
#include "attrhandlers/seqnumberattr.h"
#include "filters/categoryfilter.h"
#include "filters/duplicatefilter.h"
#include "filters/functionfilter.h"
#include "filters/regexpfilter.h"
#include "formatters/functionformatter.h"
#include "formatters/jsonformatter.h"
#include "formatters/patternformatter.h"
#include "formatters/prettyformatter.h"
#include "formatters/qtlogmessageformatter.h"
#include "messagepatterns.h"
#include "sinks/platformstdsink.h"
#include "sinks/rotatingfilesink.h"
#include "sinks/stderrsink.h"
#include "sinks/stdoutsink.h"

#ifdef QTLOGGER_NETWORK
#    include "attrhandlers/hostinfoattrs.h"
#    include "sinks/httpsink.h"
#endif

#ifdef QTLOGGER_SYSLOG
#    include "sinks/syslogsink.h"
#endif

#ifdef QTLOGGER_SDJOURNAL
#    include "sinks/sdjournalsink.h"
#endif

#ifdef Q_OS_WIN
#    include "sinks/windebugsink.h"
#endif

namespace QtLogger {

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::addSeqNumber()
{
    append(SeqNumberAttrPtr::create());
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::addAppInfo()
{
    append(AppInfoAttrsPtr::create());
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::attrHandler(std::function<QVariantHash()> func)
{
    append(FunctionAttrHandlerPtr::create(func));
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
SimplePipeline &SimplePipeline::filter(std::function<bool(const LogMessage &)> func)
{
    append(FunctionFilterPtr::create(func));
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
SimplePipeline &SimplePipeline::filter(const QString &regexp)
{
    append(RegExpFilterPtr::create(regexp));
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
SimplePipeline &SimplePipeline::formatPretty()
{
    append(PrettyFormatterPtr::create());
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::formatToJson()
{
    append(JsonFormatterPtr::create());
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToStdOut()
{
    append(StdOutSinkPtr::create());
    return *this;
}

QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToStdErr()
{
    append(StdErrSinkPtr::create());
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
                                           int maxFileCount)
{
    if (maxFileSize == 0)
        append(RotatingFileSinkPtr::create(fileName));
    else
        append(RotatingFileSinkPtr::create(fileName, maxFileSize, maxFileCount));
    return *this;
}

#ifdef QTLOGGER_NETWORK
QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToHttp(const QString &url)
{
    append(HttpSinkPtr::create(url));
    return *this;
}
#endif

#ifdef Q_OS_WIN
SimplePipeline &SimplePipeline::sendToWinDebug()
{
    append(WinDebugSinkPtr::create());
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

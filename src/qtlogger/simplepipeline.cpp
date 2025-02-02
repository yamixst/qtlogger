#include "simplepipeline.h"

#include <QCoreApplication>

#include "attrhandlers/appinfoattrs.h"
#include "attrhandlers/seqnumberattr.h"
#include "filters/functionfilter.h"
#include "filters/regexpfilter.h"
#include "formatters/functionformatter.h"
#include "formatters/jsonformatter.h"
#include "formatters/patternformatter.h"
#include "formatters/prettyformatter.h"
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

#ifdef QTLOGGER_JOURNAL
#    include "sinks/journalsink.h"
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
SimplePipeline &SimplePipeline::filterCategory(const QString &filter)
{
    // TODO: Implement
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
    else if (pattern == "pretty")
        append(PrettyFormatterPtr::create());
    else
        append(PatternFormatterPtr::create(pattern));
    return *this;
}

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

#ifdef QTLOGGER_JOURNAL
QTLOGGER_DECL_SPEC
SimplePipeline &SimplePipeline::sendToJournal()
{
    append(JournalSinkPtr::create());
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

SimplePipeline &SimplePipeline::pipeline()
{
    auto pipeline = SimplePipelinePtr::create(true, this);
    append(pipeline);
    return *pipeline.data();
}

SimplePipeline &SimplePipeline::end()
{
    if (m_parent)
        return *m_parent;
    else
        return *this;
}

} // namespace QtLogger

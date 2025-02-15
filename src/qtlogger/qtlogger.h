#include "attrhandler.h"
#include "attrhandlers/appinfoattrs.h"
#include "attrhandlers/seqnumberattr.h"
#include "filter.h"
#include "filters/categoryfilter.h"
#include "filters/duplicatefilter.h"
#include "filters/functionfilter.h"
#include "filters/regexpfilter.h"
#include "formatter.h"
#include "formatters/functionformatter.h"
#include "formatters/jsonformatter.h"
#include "formatters/patternformatter.h"
#include "formatters/prettyformatter.h"
#include "formatters/qtlogmessageformatter.h"
#include "handler.h"
#include "logger.h"
#include "logmessage.h"
#include "messagepatterns.h"
#include "pipeline.h"
#include "setmessagepattern.h"
#include "simplepipeline.h"
#include "sink.h"
#include "sinks/filesink.h"
#include "sinks/iodevicesink.h"
#include "sinks/platformstdsink.h"
#include "sinks/rotatingfilesink.h"
#include "sinks/signalsink.h"
#include "sinks/stderrsink.h"
#include "sinks/stdoutsink.h"
#include "sortedpipeline.h"
#include "version.h"

#ifdef QTLOGGER_NETWORK
#    include "attrhandlers/hostinfoattrs.h"
#    include "sinks/httpsink.h"
#endif

#ifdef Q_OS_WIN
#    include "sinks/windebugsink.h"
#endif

#ifdef QTLOGGER_IOSLOG
#    include "sinks/oslogsink.h"
#endif

#ifdef QTLOGGER_ANDROIDLOG
#    include "sinks/androidlogsink.h"
#endif

#ifdef QTLOGGER_SYSLOG
#    include "sinks/syslogsink.h"
#endif

#ifdef QTLOGGER_SDJOURNAL
#    include "sinks/sdjournalsink.h"
#endif

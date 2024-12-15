#include "abstractmessagefilter.h"
#include "abstractmessageformatter.h"
#include "messagehandler.h"
#include "abstractmessagesink.h"
#include "logmessage.h"
#include "filters/functionfilter.h"
#include "filters/regexpfilter.h"
#include "formatters/defaultformatter.h"
#include "formatters/functionformatter.h"
#include "formatters/jsonformatter.h"
#include "formatters/nullformatter.h"
#include "formatters/patternformatter.h"
#include "formatters/prettyformatter.h"
#include "formatters/qtlogmessageformatter.h"
#include "logger.h"
#include "pipelinehandler.h"
#include "messagepatterns.h"
#include "setmessagepattern.h"
#include "sinks/filesink.h"
#include "sinks/iodevicesink.h"
#include "sinks/rotatingfilesink.h"
#include "sinks/signalsink.h"
#include "sinks/stderrsink.h"
#include "sinks/stdlogsink.h"
#include "sinks/stdoutsink.h"
#include "version.h"

using QtLoggerAbstractMessageFilter = QtLogger::AbstractMessageFilter;
using QtLoggerAbstractMessageFormatter = QtLogger::AbstractMessageFormatter;
using QtLoggerMessageHandler = QtLogger::MessageHandler;
using QtLoggerAbstractMessageSink = QtLogger::AbstractMessageSink;
using QtLoggerLogMessage = QtLogger::LogMessage;
using QtLoggerDefaultFormatter = QtLogger::DefaultFormatter;
using QtLoggerFileSink = QtLogger::FileSink;
using QtLoggerFunctionFilter = QtLogger::FunctionFilter;
using QtLoggerFunctionFormatter = QtLogger::FunctionFormatter;
using QtLoggerIODeviceSink = QtLogger::IODeviceSink;
using QtLoggerJsonFormatter = QtLogger::JsonFormatter;
using QtLoggerLogger = QtLogger::Logger;
using QtLoggerPipelineHandler = QtLogger::PipelineHandler;
using QtLoggerNullFormatter = QtLogger::NullFormatter;
using QtLoggerPatternFormatter = QtLogger::PatternFormatter;
using QtLoggerPrettyFormatter = QtLogger::PrettyFormatter;
using QtLoggerQtLogMessageFormatter = QtLogger::QtLogMessageFormatter;
using QtLoggerRegExpFilter = QtLogger::RegExpFilter;
using QtLoggerRotatingFileSink = QtLogger::RotatingFileSink;
using QtLoggerSignalSink = QtLogger::SignalSink;
using QtLoggerStdErrSink = QtLogger::StdErrSink;
using QtLoggerStdLogSink = QtLogger::StdLogSink;
using QtLoggerStdOutSink = QtLogger::StdOutSink;

using QtLoggerAbstractMessageFilterPtr = QtLogger::AbstractMessageFilterPtr;
using QtLoggerAbstractMessageFormatterPtr = QtLogger::AbstractMessageFormatterPtr;
using QtLoggerMessageHandlerPtr = QtLogger::MessageHandlerPtr;
using QtLoggerAbstractMessageSinkPtr = QtLogger::AbstractMessageSinkPtr;
using QtLoggerDefaultFormatterPtr = QtLogger::DefaultFormatterPtr;
using QtLoggerFileSinkPtr = QtLogger::FileSinkPtr;
using QtLoggerFunctionFilterPtr = QtLogger::FunctionFilterPtr;
using QtLoggerFunctionFormatterPtr = QtLogger::FunctionFormatterPtr;
using QtLoggerIODeviceSinkPtr = QtLogger::IODeviceSinkPtr;
using QtLoggerJsonFormatterPtr = QtLogger::JsonFormatterPtr;
using QtLoggerPipelineHandlerPtr = QtLogger::PipelineHandlerPtr;
using QtLoggerNullFormatterPtr = QtLogger::NullFormatterPtr;
using QtLoggerPatternFormatterPtr = QtLogger::PatternFormatterPtr;
using QtLoggerPrettyFormatterPtr = QtLogger::PrettyFormatterPtr;
using QtLoggerQtLogMessageFormatterPtr = QtLogger::QtLogMessageFormatterPtr;
using QtLoggerRegExpFilterPtr = QtLogger::RegExpFilterPtr;
using QtLoggerRotatingFileSinkPtr = QtLogger::RotatingFileSinkPtr;
using QtLoggerSignalSinkPtr = QtLogger::SignalSinkPtr;
using QtLoggerStdErrSinkPtr = QtLogger::StdErrSinkPtr;
using QtLoggerStdLogSinkPtr = QtLogger::StdLogSinkPtr;
using QtLoggerStdOutSinkPtr = QtLogger::StdOutSinkPtr;

using QtLoggerFilter = QtLogger::AbstractMessageFilter;
using QtLoggerFormatter = QtLogger::AbstractMessageFormatter;
using QtLoggerHandler = QtLogger::MessageHandler;
using QtLoggerSink = QtLogger::AbstractMessageSink;
using QtLoggerPipeline = QtLogger::PipelineHandler;

using QtLoggerFilterPtr = QtLogger::AbstractMessageFilterPtr;
using QtLoggerFormatterPtr = QtLogger::AbstractMessageFormatterPtr;
using QtLoggerHandlerPtr = QtLogger::MessageHandlerPtr;
using QtLoggerSinkPtr = QtLogger::AbstractMessageSinkPtr;
using QtLoggerPipelinePtr = QtLogger::PipelineHandlerPtr;

#ifdef QTLOGGER_NETWORK
#include "sinks/httpsink.h"
using QtLoggerHttpSink = QtLogger::HttpSink;
using QtLoggerHttpSinkPtr = QtLogger::HttpSinkPtr;
#endif

#ifdef QTLOGGER_IOSLOG
#include "sinks/ioslogsink.h"
using QtLoggerIosLogSink = QtLogger::IosLogSink;
using QtLoggerIosLogSinkPtr = QtLogger::IosLogSinkPtr;
#endif

#ifdef QTLOGGER_ANDROIDLOG
#include "sinks/androidlogsink.h"
using QtLoggerAndroidLogSink = QtLogger::AndroidLogSink;
using QtLoggerAndroidLogSinkPtr = QtLogger::AndroidLogSinkPtr;
#endif

#ifdef QTLOGGER_SYSLOG
#include "sinks/syslogsink.h"
using QtLoggerSysLogSink = QtLogger::SysLogSink;
using QtLoggerSysLogSinkPtr = QtLogger::SysLogSinkPtr;
#endif

#ifdef QTLOGGER_JOURNAL
#include "sinks/journalsink.h"
using QtLoggerJournalSink = QtLogger::JournalSink;
using QtLoggerJournalSinkPtr = QtLogger::JournalSinkPtr;
#endif

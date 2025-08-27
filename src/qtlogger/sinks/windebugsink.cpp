#ifdef Q_OS_WIN

#include "windebugsink.h"

#include <qt_windows.h>

#include "../logger_global.h"

namespace QtLogger {

QTLOGGER_DECL_SPEC
void WinDebugSink::send(const LogMessage &lmsg)
{
    auto formattedMessage = lmsg.formattedMessage() + u'\n';
    OutputDebugString(reinterpret_cast<const wchar_t *>(formattedMessage.utf16()));
}

} // namespace QtLogger

#endif

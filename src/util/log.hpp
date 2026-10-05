// src/util/log.hpp
//
// Development logging. A text service runs inside other people's processes, so
// a console is not available and a debugger is often not attached; a file log is
// the only practical way to see what TSF is doing.
//
// The log is written to:
//     %LOCALAPPDATA%\Odri\odri-windows.log
//
// Logging is compiled in only when ODRI_WINDOWS_LOGGING is defined (the CMake
// option ODRI_WINDOWS_ENABLE_LOGGING, ON for Debug builds). In release builds
// the macros expand to nothing, so no host application text can ever reach disk.
#pragma once

#include <string>

namespace odri_windows::log {

enum class Level { Debug, Info, Warn, Error };

void Initialize();
void Shutdown();

void Write(Level level, const std::string& message);
void Write(Level level, const std::wstring& message);

// Text typed by the user is sensitive: it belongs to the host application, not
// to us. These are separate calls so that they are easy to grep for and easy to
// disable independently of ordinary event logging.
void WriteComposition(const char* label, const std::string& utf8_text);

}  // namespace odri_windows::log

#ifdef ODRI_WINDOWS_LOGGING
#define ODRI_LOG_DEBUG(msg) ::odri_windows::log::Write(::odri_windows::log::Level::Debug, msg)
#define ODRI_LOG_INFO(msg) ::odri_windows::log::Write(::odri_windows::log::Level::Info, msg)
#define ODRI_LOG_WARN(msg) ::odri_windows::log::Write(::odri_windows::log::Level::Warn, msg)
#define ODRI_LOG_ERROR(msg) ::odri_windows::log::Write(::odri_windows::log::Level::Error, msg)
#define ODRI_LOG_TEXT(label, utf8) ::odri_windows::log::WriteComposition(label, utf8)
#else
#define ODRI_LOG_DEBUG(msg) ((void)0)
#define ODRI_LOG_INFO(msg) ((void)0)
#define ODRI_LOG_WARN(msg) ((void)0)
#define ODRI_LOG_ERROR(msg) ((void)0)
#define ODRI_LOG_TEXT(label, utf8) ((void)0)
#endif

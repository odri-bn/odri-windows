// src/util/log.cpp
#include "util/log.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <knownfolders.h>
#include <shlobj.h>

#include <cstdio>
#include <mutex>

#include "windows/unicode.hpp"

namespace odri_windows::log
{
    namespace
    {

        std::mutex g_mutex;
        std::wstring g_path;
        bool g_initialized = false;

        std::wstring DefaultLogPath()
        {
            PWSTR local_appdata = nullptr;
            if (FAILED(::SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local_appdata)))
                return {};
            std::wstring dir(local_appdata);
            ::CoTaskMemFree(local_appdata);

            dir += L"\\Odri";
            ::CreateDirectoryW(dir.c_str(), nullptr);
            return dir + L"\\odri-windows.log";
        }

        const char *LevelName(Level level)
        {
            switch (level)
            {
            case Level::Debug:
                return "DEBUG";
            case Level::Info:
                return "INFO ";
            case Level::Warn:
                return "WARN ";
            case Level::Error:
                return "ERROR";
            }
            return "?????";
        }

        void WriteLine(Level level, const std::string &message)
        {
#ifndef ODRI_WINDOWS_LOGGING
            (void)level;
            (void)message;
#else
            std::lock_guard<std::mutex> guard(g_mutex);
            if (!g_initialized)
                return;
            if (g_path.empty())
                return;

            SYSTEMTIME now{};
            ::GetLocalTime(&now);

            char prefix[64] = {};
            std::snprintf(prefix, sizeof(prefix), "%04u-%02u-%02u %02u:%02u:%02u.%03u [%5u] %s ",
                          now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond,
                          now.wMilliseconds, ::GetCurrentThreadId(), LevelName(level));

            std::string line = prefix + message + "\n";
            HANDLE file = ::CreateFileW(g_path.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (file == INVALID_HANDLE_VALUE)
                return;

            DWORD written = 0;
            ::WriteFile(file, line.data(), static_cast<DWORD>(line.size()), &written, nullptr);
            ::CloseHandle(file);
#endif
        }

    } // namespace

    void Initialize()
    {
        std::lock_guard<std::mutex> guard(g_mutex);
        if (g_initialized)
            return;
        g_path = DefaultLogPath();
        g_initialized = true;
    }

    void Shutdown()
    {
        std::lock_guard<std::mutex> guard(g_mutex);
        g_initialized = false;
        g_path.clear();
    }

    void Write(Level level, const std::string &message) { WriteLine(level, message); }

    void Write(Level level, const std::wstring &message)
    {
        WriteLine(level, WideToUtf8(message));
    }

    void WriteComposition(const char *label, const std::string &utf8_text)
    {
        WriteLine(Level::Debug, std::string(label) + ": " + utf8_text);
    }

} // namespace odri_windows::log

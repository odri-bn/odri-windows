#pragma once

#include <odri/odri.h>

#include <string>
#include <string_view>

namespace okkhor_windows
{

    class EngineHost
    {
    public:
        EngineHost();
        ~EngineHost();

        EngineHost(EngineHost &&) noexcept;
        EngineHost &operator=(EngineHost &&) noexcept;

        EngineHost(const EngineHost &) = delete;
        EngineHost &operator=(const EngineHost &) = delete;

        bool Transliterate(
            std::string_view latin_utf8,
            std::string *out) const;

    private:
        OdriEngine *engine_ = nullptr;
    };

} // namespace okkhor_windows
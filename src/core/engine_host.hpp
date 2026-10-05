#pragma once

#include <odri/odri.h>

#include <string>
#include <string_view>
#include <vector>

namespace okkhor_windows
{

    struct EngineUnit
    {
        unsigned int kind;
        std::string text;

        bool operator==(const EngineUnit &other) const
        {
            return kind == other.kind &&
                   text == other.text;
        }
    };

    class EngineHost
    {
    public:
        EngineHost();
        ~EngineHost();

        EngineHost(const EngineHost &) = delete;
        EngineHost &operator=(const EngineHost &) = delete;

        EngineHost(EngineHost &&other) noexcept;
        EngineHost &operator=(EngineHost &&other) noexcept;

        bool Transliterate(
            std::string_view latin_utf8,
            std::string *out) const;

        bool ConvertUnits(
            std::string_view latin_utf8,
            std::vector<EngineUnit> *out) const;

    private:
        OdriEngine *engine_ = nullptr;
    };

} // namespace okkhor_windows
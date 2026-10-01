#include "core/engine_host.hpp"

#include <utility>

namespace okkhor_windows
{

    EngineHost::EngineHost()
        : engine_(odri_engine_new())
    {
    }

    EngineHost::~EngineHost()
    {
        if (engine_)
        {
            odri_engine_free(engine_);
        }
    }

    EngineHost::EngineHost(EngineHost &&other) noexcept
        : engine_(std::exchange(other.engine_, nullptr))
    {
    }

    EngineHost &EngineHost::operator=(EngineHost &&other) noexcept
    {
        if (this != &other)
        {
            if (engine_)
            {
                odri_engine_free(engine_);
            }

            engine_ = std::exchange(other.engine_, nullptr);
        }

        return *this;
    }

    bool EngineHost::Transliterate(
        std::string_view latin_utf8,
        std::string *out) const
    {
        if (!out || !engine_)
        {
            return false;
        }

        char *result = odri_engine_convert(
            engine_,
            latin_utf8.data());

        if (!result)
        {
            return false;
        }

        *out = result;

        odri_string_free(result);

        return true;
    }
} // namespace okkhor_windows
#include "core/engine_host.hpp"

#include <utility>

namespace odri_windows
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

EngineHost::EngineHost(EngineHost&& other) noexcept
    : engine_(std::exchange(other.engine_, nullptr))
{
}

EngineHost& EngineHost::operator=(EngineHost&& other) noexcept
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
    std::string* out
) const
{
    if (!out || !engine_)
    {
        return false;
    }

    std::string input(latin_utf8);

    char* result = odri_engine_convert(
        engine_,
        input.c_str()
    );

    if (!result)
    {
        return false;
    }

    // Copy the Rust-owned string before freeing it.
    *out = result;

    odri_string_free(result);

    return true;
}

bool EngineHost::ConvertUnits(
    std::string_view latin_utf8,
    std::vector<EngineUnit>* out
) const
{
    if (!out || !engine_)
    {
        return false;
    }

    out->clear();

    std::string input(latin_utf8);

    OdriUnitArray array{};

    if (!odri_engine_convert_units(
            engine_,
            input.c_str(),
            &array))
    {
        return false;
    }

    out->reserve(array.len);

    for (size_t i = 0; i < array.len; ++i)
    {
        const OdriUnit& unit = array.units[i];

        EngineUnit result;
        result.kind = unit.kind;

        if (unit.text)
        {
            result.text = unit.text;
        }

        out->push_back(std::move(result));
    }

    // This releases both the OdriUnit array
    // and every Rust-owned unit.text.
    odri_units_free(array);

    return true;
}

} // namespace odri_windows
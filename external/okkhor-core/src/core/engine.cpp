#include <okkhor/okkhor.hpp>

#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

// Internal implementation
#include "mapping.hpp"
#include "orthography.hpp"
#include "token.hpp"
#include "tokenizer.hpp"

#include "bangla_to_latin/index.hpp"
#include "latin_to_bangla/index.hpp"

#include "util/json.hpp"
#include "default_data.hpp"

namespace okkhor {

namespace {

// -----------------------------------------------------------------------------
// File helpers
// -----------------------------------------------------------------------------

std::string read_file(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        throw std::runtime_error(
            "could not open Okkhor data file: " + path
        );
    }

    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

std::string data_path(
    const std::string& data_dir,
    const std::string& filename
)
{
    if (data_dir.empty()) {
        return filename;
    }

    return data_dir + "/" + filename;
}

} // namespace


// =============================================================================
// Engine::Impl
// =============================================================================

class Engine::Impl {
public:

    Mapping mapping;

    latin_to_bangla::RuleEngine latin_to_bangla_rules;
    bangla_to_latin::RuleEngine bangla_to_latin_rules;

    latin_to_bangla::Renderer latin_to_bangla_renderer;
    bangla_to_latin::Renderer bangla_to_latin_renderer;


    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    Impl()
        : mapping(
            Mapping::load(
                json::parse(default_data::vowels),
                json::parse(default_data::consonants),
                json::parse(default_data::controls),
                json::parse(default_data::punctuation)
            )
        ),
          latin_to_bangla_rules(),
          bangla_to_latin_rules(),
          latin_to_bangla_renderer(mapping),
          bangla_to_latin_renderer(mapping)
    {
        const auto rules =
            json::parse(default_data::rules);

        latin_to_bangla_rules.load(
            rules,
            mapping
        );

        bangla_to_latin_rules.load(
            rules,
            mapping
        );
    }


    explicit Impl(const std::string& data_dir)
        : mapping(
            Mapping::load(
                json::parse(
                    read_file(
                        data_path(data_dir, "vowels.json")
                    )
                ),
                json::parse(
                    read_file(
                        data_path(data_dir, "consonants.json")
                    )
                ),
                json::parse(
                    read_file(
                        data_path(data_dir, "controls.json")
                    )
                ),
                json::parse(
                    read_file(
                        data_path(data_dir, "punctuation.json")
                    )
                )
            )
        ),
          latin_to_bangla_rules(),
          bangla_to_latin_rules(),
          latin_to_bangla_renderer(mapping),
          bangla_to_latin_renderer(mapping)
    {
        const auto rules =
            json::parse(
                read_file(
                    data_path(data_dir, "rules.json")
                )
            );

        latin_to_bangla_rules.load(
            rules,
            mapping
        );

        bangla_to_latin_rules.load(
            rules,
            mapping
        );
    }
};


// =============================================================================
// Engine
// =============================================================================

Engine::Engine()
    : impl_(std::make_unique<Impl>())
{
}

Engine::Engine(const std::string& data_dir)
    : impl_(std::make_unique<Impl>(data_dir))
{
}

Engine::~Engine() = default;


// -----------------------------------------------------------------------------
// Move operations
// -----------------------------------------------------------------------------

Engine::Engine(Engine&&) noexcept = default;

Engine& Engine::operator=(Engine&&) noexcept = default;


// -----------------------------------------------------------------------------
// Latin -> Bangla
// -----------------------------------------------------------------------------

std::string Engine::transliterate_latin_to_bangla(
    std::string_view input
) const
{
    std::string text(input);

    auto raw_tokens =
        tokenize(
            text,
            impl_->mapping,
            WorkingDirection::Forward
        );

    auto rewritten_tokens =
        impl_->latin_to_bangla_rules.apply(
            raw_tokens,
            impl_->mapping
        );

    Document document =
        latin_to_bangla::parse(
            rewritten_tokens,
            impl_->mapping
        );

    return impl_->latin_to_bangla_renderer.render(
        document
    );
}


// -----------------------------------------------------------------------------
// Bangla -> Latin
// -----------------------------------------------------------------------------

std::string Engine::transliterate_bangla_to_latin(
    std::string_view input
) const
{
    std::string text(input);

    auto raw_tokens =
        tokenize(
            text,
            impl_->mapping,
            WorkingDirection::Reverse
        );

    // Currently the reverse tokenizer/parser pipeline
    // does not perform a token rewrite stage.
    auto rewritten_tokens = raw_tokens;

    Document document =
        bangla_to_latin::parse(
            rewritten_tokens,
            impl_->mapping
        );

    std::string result =
        impl_->bangla_to_latin_renderer.render(
            document
        );

    return impl_->bangla_to_latin_rules.apply(
        result
    );
}

} // namespace okkhor
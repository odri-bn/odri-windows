#pragma once

#include <string>

#include "core/mapping.hpp"
#include "core/orthography.hpp"

namespace okkhor::bangla_to_latin {

class Renderer {
public:
    explicit Renderer(const Mapping& mapping)
        : mapping_(&mapping) {}

    std::string render(const OrthographicUnit& unit) const;
    std::string render(const IndependentVowel& vowel) const;
    std::string render(const Element& element) const;
    std::string render(const Document& document) const;

private:
    const Mapping* mapping_;
};

} // namespace okkhor::bangla_to_latin
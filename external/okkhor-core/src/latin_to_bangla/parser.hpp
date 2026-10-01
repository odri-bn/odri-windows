#pragma once
// Phase 5: tokens -> algebraic operations. The parser never touches Unicode.
#include <vector>

#include "core/orthography.hpp"
#include "core/tokenizer.hpp"
#include "core/mapping.hpp"

namespace okkhor::latin_to_bangla {

Document parse(const std::vector<Token> &tokens, const Mapping &mapping);

} // namespace okkhor

#pragma once

#include <vector>

#include "core/orthography.hpp"
#include "core/token.hpp"
#include "core/mapping.hpp"

namespace okkhor::bangla_to_latin {

Document parse(const std::vector<Token> &tokens, const Mapping &mapping);

} // namespace okkhor::bangla_to_latin
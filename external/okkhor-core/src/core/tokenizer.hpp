#pragma once

// Phase 4: Latin phonetic input -> semantic tokens.
// Longest match wins.

#include "core/mapping.hpp"
#include "core/token.hpp"
#include <string>
#include <vector>

namespace okkhor {
enum class WorkingDirection { Forward, Reverse };

class Mapping;

std::vector<Token> tokenize(const std::string &input, const Mapping &mapping,
                            WorkingDirection direction);

const char *token_type_name(TokenType t);

} // namespace okkhor
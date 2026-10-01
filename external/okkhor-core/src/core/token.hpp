#pragma once
#include <string>

namespace okkhor {

enum class TokenType {
  Vowel,
  Consonant,
  VirtualConsonant,
  Accent,

  Hasanta,
  ZWNJ,
  ZWJ,

  Whitespace,
  Punctuation,
  Special,
  Unknown
};

struct Token {
  TokenType type = TokenType::Unknown;
  std::string canonical_key;
  std::string value;
};

} // namespace okkhor
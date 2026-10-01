#include "tokenizer.hpp"

#include <algorithm>
#include <cctype>

#include "core/mapping.hpp"

namespace okkhor {
namespace {

bool is_space(unsigned char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' ||
         c == '\f';
}

// Length of the UTF-8 sequence starting at `c`.
// Unknown multi-byte input survives intact rather than being
// split into individual bytes.
std::size_t utf8_length(unsigned char c) {
  if ((c & 0x80) == 0x00)
    return 1;
  if ((c & 0xE0) == 0xC0)
    return 2;
  if ((c & 0xF0) == 0xE0)
    return 3;
  if ((c & 0xF8) == 0xF0)
    return 4;
  return 1;
}

} // namespace

const char *token_type_name(TokenType t) {
  switch (t) {
  case TokenType::Vowel:
    return "Vowel";
  case TokenType::Consonant:
    return "Consonant";
  case TokenType::VirtualConsonant:
    return "VirtualConsonant";
  case TokenType::Accent:
    return "Accent";
  case TokenType::Hasanta:
    return "Hasanta";
  case TokenType::ZWNJ:
    return "ZWNJ";
  case TokenType::ZWJ:
    return "ZWJ";
  case TokenType::Whitespace:
    return "Whitespace";
  case TokenType::Punctuation:
    return "Punctuation";
  case TokenType::Special:
    return "Special";
  case TokenType::Unknown:
    return "Unknown";
  }
  return "Unknown";
}

std::vector<Token> tokenize(const std::string &input, const Mapping &mapping,
                            WorkingDirection direction) {
  std::vector<Token> out;
  const std::size_t n = input.size();
  const std::size_t max_len = mapping.max_key_length();
  std::size_t i = 0;

  while (i < n) {
    // Longest match against mappings available
    // in the requested direction.
    bool matched = false;
    const std::size_t limit = std::min(max_len, n - i);

    for (std::size_t len = limit; len >= 1; --len) {
      const std::string key = input.substr(i, len);
      const Rule *rule = mapping.lookup(
          key, direction == WorkingDirection::Forward ? Direction::Forward
                                                      : Direction::Reverse);

      if (!rule)
        continue;

      Token token;
      token.type = rule->type;
      token.canonical_key = rule->canonical_key;
      token.value = key;

      out.push_back(std::move(token));
      i += len;
      matched = true;
      break;
    }

    if (matched)
      continue;

    // Preserve whitespace separately.
    const unsigned char c = static_cast<unsigned char>(input[i]);
    if (is_space(c)) {
      Token token;
      token.type = TokenType::Whitespace;
      token.value = std::string(1, input[i]);
      out.push_back(std::move(token));
      ++i;
      continue;
    }

    // Unknown input is preserved verbatim.
    // For UTF-8 input, consume the complete code point
    // instead of splitting it into individual bytes.
    const std::size_t len = std::min(utf8_length(c), n - i);

    Token token;
    token.type = TokenType::Unknown;
    token.value = input.substr(i, len);

    out.push_back(std::move(token));
    i += len;
  }

  return out;
}

} // namespace okkhor
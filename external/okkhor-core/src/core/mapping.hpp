#pragma once

// Phase 0: data-driven mapping tables (data/*.json).

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <bitset>

#include "core/token.hpp"
#include "util/json.hpp"

namespace okkhor {

enum class Direction : uint8_t {
  None = 0,
  Forward = 1 << 0,
  Reverse = 1 << 1,
  Both = Forward | Reverse
};

constexpr bool supports(Direction available, Direction requested) {
  return (static_cast<uint8_t>(available) & static_cast<uint8_t>(requested)) !=
         0;
}

struct VowelEntry {
  std::string canonical_key; // canonical Latin key
  std::string independent;   // "অ", "আ", ...
  std::string dependent;     // "", "া", ...
  std::vector<std::string> aliases;
};

struct ConsonantEntry {
  std::string canonical_key; // canonical Latin key
  std::string base;          // "ক"
  std::string fola;          // optional rendering of dcons inside conjunct
  std::vector<std::string> aliases;
};

struct OtherEntry {
  std::string canonical_key; // canonical Latin key
  std::string value;         // "ং", "ঁ", "ঃ"
  std::vector<std::string> aliases;
};

// What a matched Latin key means.
struct Rule {
  TokenType type = TokenType::Unknown;
  Direction direction = Direction::Both;
  std::string canonical_key;
  std::string value;
  std::string literal;
};

class Mapping {
public:
  // Loads vowels.json, consonants.json, controls.json,
  // and punctuation.json.
  static Mapping load(const json::Value &vowels_json,
                      const json::Value &consonants_json,
                      const json::Value &controls_json,
                      const json::Value &punctuation_json);

  // Looks up a Latin key, including aliases.
  const Rule *lookup(const std::string &key, Direction direction) const;

  std::size_t max_key_length() const { return max_key_len_; }

  // Canonical-key lookup.
  const VowelEntry *vowel(const std::string &canonical_key) const;
  const ConsonantEntry *consonant(const std::string &canonical_key) const;
  const OtherEntry *other(const std::string &canonical_key) const;

  const std::unordered_map<std::string, VowelEntry> &vowels() const {
    return vowels_;
  }

  const std::unordered_map<std::string, ConsonantEntry> &consonants() const {
    return consonants_;
  }

  const std::unordered_map<std::string, OtherEntry> &others() const {
    return others_;
  }

  const std::unordered_map<std::string, std::string> &controls() const {
    return controls_;
  }

  void add_rule(const std::string &key, Rule rule);

  void set_rule(const std::string &key, Rule rule);

private:
  std::unordered_map<std::string, Rule> rules_;

  // Keyed by canonical Latin representation.
  std::unordered_map<std::string, VowelEntry> vowels_;
  std::unordered_map<std::string, ConsonantEntry> consonants_;
  std::unordered_map<std::string, OtherEntry> others_;

  std::unordered_map<std::string, std::string> controls_;

  std::size_t max_key_len_ = 1;
};

} // namespace okkhor
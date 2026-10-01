#include "mapping.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "tokenizer.hpp"
#include "util/json.hpp"

namespace okkhor {

namespace {

std::string join(const std::string &dir, const char *file) {
  if (dir.empty())
    return file;

  char last = dir.back();

  if (last == '/' || last == '\\')
    return dir + file;

  return dir + "/" + file;
}

std::string capitalize_first_ascii(const std::string &key) {
  if (key.empty())
    return key;

  std::string result = key;

  if (result[0] >= 'a' && result[0] <= 'z')
    result[0] = static_cast<char>(result[0] - 'a' + 'A');

  return result;
}

} // namespace

void Mapping::add_rule(const std::string &key, Rule rule) {
  // std::cout << "Mapping::add_rule: "
  //           << "direction="
  //           << std::bitset<2>(static_cast<uint8_t>(rule.direction))
  //           << " key=" << key << " type=" << token_type_name(rule.type)
  //           << " canonical=" << rule.canonical_key << " value=" << rule.value
  //           << '\n';

  if (key.empty())
    throw std::runtime_error("empty key in mapping data");

  if (rules_.count(key))
    throw std::runtime_error("duplicate mapping key: " + key);

  max_key_len_ = std::max(max_key_len_, key.size());

  rules_.emplace(key, std::move(rule));
}

void Mapping::set_rule(const std::string &key, Rule rule) {
  if (key.empty())
    throw std::runtime_error("empty key in mapping data");
  max_key_len_ = std::max(max_key_len_, key.size());
  rules_[key] = std::move(rule);
}

const Rule *Mapping::lookup(const std::string &key, Direction direction) const {
  auto it = rules_.find(key);
  if (it == rules_.end())
    return nullptr;
  if (!supports(it->second.direction, direction))
    return nullptr;
  return &it->second;
}

const VowelEntry *Mapping::vowel(const std::string &canonical_key) const {
  auto it = vowels_.find(canonical_key);

  if (it == vowels_.end())
    throw std::out_of_range("unknown vowel key: " + canonical_key);

  return &it->second;
}

const ConsonantEntry *
Mapping::consonant(const std::string &canonical_key) const {
  auto it = consonants_.find(canonical_key);

  if (it == consonants_.end())
    throw std::out_of_range("unknown consonant key: " + canonical_key);

  return &it->second;
}

const OtherEntry *Mapping::other(const std::string &canonical_key) const {
  auto it = others_.find(canonical_key);

  if (it == others_.end())
    throw std::out_of_range("unknown other key: " + canonical_key);

  return &it->second;
}

Mapping Mapping::load(const json::Value &vowels_json,
                      const json::Value &consonants_json,
                      const json::Value &controls_json,
                      const json::Value &punctuation_json) {

  Mapping m;

  // =========================================================
  // vowels.json
  // =========================================================

  {
    json::Value doc = vowels_json;

    if (!doc.is_object())
      throw std::runtime_error("vowels.json: expected an object");

    // -----------------------------------------------------
    // Pass 1: canonical entries
    // -----------------------------------------------------

    for (const auto &[key, spec] : doc.as_object()) {
      VowelEntry e;

      e.canonical_key = key;
      e.independent = spec.string_or("ind", "");
      e.dependent = spec.string_or("dep", "");

      if (m.vowels_.count(key))
        throw std::runtime_error("duplicate vowel key: " + key);

      m.vowels_.emplace(key, e);

      // Canonical Latin key.
      m.add_rule(key, Rule{TokenType::Vowel, Direction::Forward, key, "", ""});

      // Bengali independent vowel.
      if (!e.independent.empty()) {
        m.add_rule(e.independent,
                   Rule{TokenType::Vowel, Direction::Reverse, key, "", ""});
      }

      // Bengali dependent vowel sign.
      if (!e.dependent.empty()) {
        m.add_rule(e.dependent,
                   Rule{TokenType::Vowel, Direction::Reverse, key, "", ""});
      }
    }

    // -----------------------------------------------------
    // Pass 2: explicit aliases
    // -----------------------------------------------------
    for (const auto &[key, spec] : doc.as_object()) {
      const json::Value *aliases = spec.find("aliases");

      if (!aliases || !aliases->is_array())
        continue;

      for (const auto &alias : aliases->as_array()) {
        if (!alias.is_string())
          throw std::runtime_error("vowels.json: alias must be a string");

        const std::string &alias_key = alias.as_string();

        m.add_rule(alias_key,
                   Rule{TokenType::Vowel, Direction::Forward, key, "", ""});
      }
    }

    // -----------------------------------------------------
    // Pass 3: automatic capitalized aliases
    //
    // Only create the alias if the key is still unused.
    // Explicit canonical keys and explicit aliases always
    // take precedence.
    // -----------------------------------------------------

    for (const auto &[key, spec] : doc.as_object()) {
      (void)spec;

      std::string capitalized = capitalize_first_ascii(key);

      if (capitalized != key && !m.rules_.count(capitalized)) {

        m.add_rule(capitalized,
                   Rule{TokenType::Vowel, Direction::Forward, key, "", ""});
      }
    }
  }

  // =========================================================
  // consonants.json
  // =========================================================
  {
    json::Value doc = consonants_json;

    if (!doc.is_object())
      throw std::runtime_error("consonants.json: expected an object");

    // -----------------------------------------------------
    // Pass 1: canonical entries
    // -----------------------------------------------------

    for (const auto &[key, spec] : doc.as_object()) {
      ConsonantEntry e;

      e.canonical_key = key;
      e.base = spec.string_or("base", "");
      // e.fola = spec.string_or("fola", "");

      if (m.consonants_.count(key))
        throw std::runtime_error("duplicate consonant key: " + key);

      m.consonants_.emplace(key, e);

      // Canonical Latin key.
      m.add_rule(key, Rule{TokenType::Consonant, Direction::Forward, key, ""});

      // Bengali consonant base.
      if (!e.base.empty() && !m.rules_.count(e.base)) {
        m.add_rule(e.base,
                   Rule{TokenType::Consonant, Direction::Reverse, key, ""});
      }
    }

    // -----------------------------------------------------
    // Pass 2: explicit aliases
    // -----------------------------------------------------

    for (const auto &[key, spec] : doc.as_object()) {
      const json::Value *aliases = spec.find("aliases");

      if (!aliases || !aliases->is_array())
        continue;

      for (const auto &alias : aliases->as_array()) {
        if (!alias.is_string())
          throw std::runtime_error("consonants.json: alias must be a string");

        const std::string &alias_key = alias.as_string();

        m.add_rule(alias_key,
                   Rule{TokenType::Consonant, Direction::Forward, key, "", ""});
      }
    }

    // -----------------------------------------------------
    // Pass 3: automatic capitalized aliases
    // -----------------------------------------------------

    for (const auto &[key, spec] : doc.as_object()) {
      (void)spec;

      std::string capitalized = capitalize_first_ascii(key);

      if (capitalized != key && !m.rules_.count(capitalized)) {

        m.add_rule(capitalized,
                   Rule{TokenType::Consonant, Direction::Forward, key, "", ""});
      }
    }
  }

  // =========================================================
  // controls.json
  // =========================================================

  {
    json::Value doc = controls_json;

    if (!doc.is_object())
      throw std::runtime_error("controls.json: expected an object");

    auto simple = [&](const char *section, TokenType type) {
      const json::Value *v = doc.find(section);

      if (!v || !v->is_object())
        return;

      for (const auto &[key, out] : v->as_object()) {
        if (!out.is_string())
          throw std::runtime_error(
              std::string(
                  "controls.json: expected string output in section: ") +
              section);

        const std::string &value = out.as_string();

        // Forward representation:
        // Latin/control syntax -> semantic token

        OtherEntry e;
        e.canonical_key = key;
        e.value = value;

        if (m.others_.count(key))
          throw std::runtime_error("duplicate control key: " + key);
        m.others_.emplace(key, e);
        m.add_rule(key, Rule{type, Direction::Forward, key, "", ""});
        m.controls_.emplace(section, key);

        // Reverse representation:
        // Bangla Unicode -> same semantic token
        if (!value.empty() && value != key) {
          m.add_rule(value, Rule{type, Direction::Reverse, key, "", ""});
        }
      }
    };

    simple("hasanta", TokenType::Hasanta);
    simple("zwnj", TokenType::ZWNJ);
    simple("zwj", TokenType::ZWJ);
    simple("virtual", TokenType::VirtualConsonant);

    // -----------------------------------------------------
    // Accents
    // -----------------------------------------------------

    if (const json::Value *acc = doc.find("accents"); acc && acc->is_object()) {

      // Pass 1: canonical accent entries

      for (const auto &[key, spec] : acc->as_object()) {

        std::string value = spec.string_or("sign", "");

        OtherEntry e;

        e.canonical_key = key;
        e.value = value;

        if (m.others_.count(key))
          throw std::runtime_error("duplicate accent key: " + key);

        m.others_.emplace(key, e);
        m.add_rule(key,
                   Rule{TokenType::Accent, Direction::Forward, key, "", ""});

        if (!e.value.empty() && e.value != key) {
          m.add_rule(e.value,
                     Rule{TokenType::Accent, Direction::Reverse, key, "", ""});
        }
      }

      // Pass 2: explicit aliases

      for (const auto &[key, spec] : acc->as_object()) {

        const json::Value *aliases = spec.find("aliases");

        if (!aliases || !aliases->is_array())
          continue;

        for (const auto &alias : aliases->as_array()) {

          if (!alias.is_string())
            throw std::runtime_error("controls.json: "
                                     "accent alias must be a string");

          const std::string &alias_key = alias.as_string();

          m.add_rule(alias_key,
                     Rule{TokenType::Accent, Direction::Forward, key, "", ""});
        }
      }

      // Pass 3: automatic capitalized aliases

      for (const auto &[key, spec] : acc->as_object()) {

        (void)spec;

        std::string capitalized = capitalize_first_ascii(key);

        if (capitalized != key && !m.rules_.count(capitalized)) {

          m.add_rule(capitalized,
                     Rule{TokenType::Accent, Direction::Forward, key, "", ""});
        }
      }
    }
  }

  // =========================================================
  // punctuation.json
  // =========================================================

  {
    json::Value doc = punctuation_json;

    if (!doc.is_object())
      throw std::runtime_error("punctuation.json: expected an object");
    for (const auto &[key, out] : doc.as_object()) {

      OtherEntry e;
      e.canonical_key = key;
      e.value = out.as_string();

      m.others_.emplace(key, e);

      m.add_rule(key,
                 Rule{TokenType::Punctuation, Direction::Forward, key, "", ""});

      if (!e.value.empty() && e.value != key)
        m.add_rule(e.value, Rule{TokenType::Punctuation, Direction::Reverse,
                                 key, "", ""});
    }
  }

  return m;
}

} // namespace okkhor
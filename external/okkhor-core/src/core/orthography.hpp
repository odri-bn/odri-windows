#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace okkhor {

struct Accent {
  std::string key;
};

struct Consonant {
  std::string key;
  std::vector<Accent> accents;
};

struct Vowel {
  std::string key;
  std::vector<Accent> accents;
};

inline bool operator==(const Vowel &a, const Vowel &b) {
  return a.key == b.key && a.accents == b.accents;
}

inline bool operator==(const Consonant &a, const Consonant &b) {
  return a.key == b.key && a.accents == b.accents;
}

inline bool operator==(const Accent &a, const Accent &b) {
  return a.key == b.key;
}

struct BaseConsonant {
  Consonant value;
};

struct VirtualConsonant {};

using Base = std::variant<BaseConsonant, VirtualConsonant>;

struct DependentConsonant {
  Consonant value;
};

struct DependentVowel {
  Vowel value;
};

struct OrthographicUnit {
  Base base{VirtualConsonant{}};

  std::vector<DependentConsonant> conjuncts;

  std::optional<DependentVowel> vowel;

  bool explicit_hasanta = false;
  bool zwnj_after = false;
  bool zwj_after = false;
};

struct IndependentVowel {
  Vowel value;

  bool zwnj_after = false;
  bool zwj_after = false;
};

struct Literal {
  std::string text;
};

using Element = std::variant<OrthographicUnit, IndependentVowel, Literal>;

using Document = std::vector<Element>;

// ---- Orthographic algebra --------------------------------------------------

OrthographicUnit make_consonant(Consonant c);

OrthographicUnit make_virtual_consonant();

bool add_vowel(OrthographicUnit &u, Vowel v);

bool add_consonant(OrthographicUnit &u, Consonant c);

bool add_accent(OrthographicUnit &u, Accent a);

bool terminate_with_hasanta(OrthographicUnit &u);

void add_hasanta(OrthographicUnit &u);

void add_zwnj(OrthographicUnit &u);

void add_zwj(OrthographicUnit &u);

bool is_open(const OrthographicUnit &u);

bool is_virtual(const OrthographicUnit &u);

} // namespace okkhor
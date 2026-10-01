#include "orthography.hpp"

#include <utility>

namespace okkhor {

OrthographicUnit make_consonant(Consonant c) {
  OrthographicUnit u;

  u.base = BaseConsonant{std::move(c)};

  return u;
}

OrthographicUnit make_virtual_consonant() {
  OrthographicUnit u;

  u.base = VirtualConsonant{};

  return u;
}

bool is_virtual(const OrthographicUnit &u) {
  return std::holds_alternative<VirtualConsonant>(u.base);
}

static bool sealed(const OrthographicUnit &u) {
  return u.explicit_hasanta || u.zwnj_after || u.zwj_after;
}

bool is_open(const OrthographicUnit &u) { return !sealed(u) && !u.vowel; }

bool add_vowel(OrthographicUnit &u, Vowel v) {
  if (!is_open(u))
    return false;

  u.vowel = DependentVowel{std::move(v)};

  return true;
}

bool add_consonant(OrthographicUnit &u, Consonant c) {
  if (!is_open(u))
    return false;

  u.conjuncts.push_back(DependentConsonant{std::move(c)});

  return true;
}

bool add_accent(OrthographicUnit &u, Accent a) {
  if (sealed(u))
    return false;

  /*
   * The accent belongs to the most recently
   * created character in the orthographic unit.
   *
   * Priority:
   *
   *   vowel
   *      ↓
   *   last conjunct
   *      ↓
   *   base consonant
   *
   * Examples:
   *
   *   z ,.
   *   → base z + nukta
   *
   *   z z ,.
   *   → base z + conjunct z + nukta on second z
   *
   *   z a ^
   *   → base z + vowel a + accent ^
   */

  if (u.vowel) {
    u.vowel->value.accents.push_back(std::move(a));

    return true;
  }

  if (!u.conjuncts.empty()) {
    u.conjuncts.back().value.accents.push_back(std::move(a));

    return true;
  }

  if (auto *base = std::get_if<BaseConsonant>(&u.base)) {

    base->value.accents.push_back(std::move(a));

    return true;
  }

  return false;
}

bool terminate_with_hasanta(OrthographicUnit &u) {
  if (sealed(u) || u.vowel)
    return false;

  u.explicit_hasanta = true;

  return true;
}

void add_hasanta(OrthographicUnit &u) { u.explicit_hasanta = true; }

void add_zwnj(OrthographicUnit &u) { u.zwnj_after = true; }

void add_zwj(OrthographicUnit &u) { u.zwj_after = true; }

} // namespace okkhor
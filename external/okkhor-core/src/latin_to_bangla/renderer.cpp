#include "renderer.hpp"

#include <type_traits>

namespace okkhor::latin_to_bangla {

// dcons = hosonto + bcons.
//
// A fola form, when the data supplies one, is just another rendering
// of the same dcons -- never a separate structural type.
std::string Renderer::render_dependent(const DependentConsonant &dc) const {
  const ConsonantEntry *e = mapping_->consonant(dc.value.key);

  std::string out;

  if (!e->fola.empty()) {
    out += e->fola;
  } else {
    out += kHasanta;
    out += e->base;
  }

  // Accents belong to this consonant.
  for (const Accent &a : dc.value.accents) {
    const OtherEntry *accent = mapping_->other(a.key);

    if (accent) {
      out += accent->value;
    }
  }

  return out;
}

std::string Renderer::render(const OrthographicUnit &unit) const {
  std::string out;

  // ---------------------------------------------------------------------
  // Base
  // ---------------------------------------------------------------------
  //
  // bcons renders visibly.
  // vcons renders as nothing.

  if (const auto *bc = std::get_if<BaseConsonant>(&unit.base)) {

    const ConsonantEntry *e = mapping_->consonant(bc->value.key);

    out += e->base;

    // Accents belong to the base consonant.
    for (const Accent &a : bc->value.accents) {
      const OtherEntry *accent = mapping_->other(a.key);

      if (accent) {
        out += accent->value;
      }
    }
  }

  // ---------------------------------------------------------------------
  // Conjuncts
  // ---------------------------------------------------------------------

  for (const DependentConsonant &dc : unit.conjuncts) {
    out += render_dependent(dc);
  }

  // ---------------------------------------------------------------------
  // Vowel
  // ---------------------------------------------------------------------
  //
  // The inherent অ is a semantic state, not a character.
  // The dependent form of `o` is empty, so nothing is emitted for it.

  if (unit.vowel) {
    const VowelEntry *e = mapping_->vowel(unit.vowel->value.key);

    out += e->dependent;

    // Accents belong to the vowel.
    for (const Accent &a : unit.vowel->value.accents) {

      const OtherEntry *accent = mapping_->other(a.key);

      if (accent) {
        out += accent->value;
      }
    }
  }

  // ---------------------------------------------------------------------
  // Explicit hasanta
  // ---------------------------------------------------------------------

  if (unit.explicit_hasanta) {
    out += kHasanta;
  }

  // ---------------------------------------------------------------------
  // Join controls
  // ---------------------------------------------------------------------

  if (unit.zwnj_after) {
    out += kZwnj;
  }

  if (unit.zwj_after) {
    out += kZwj;
  }

  return out;
}

std::string Renderer::render(const IndependentVowel &v) const {
  const VowelEntry *e = mapping_->vowel(v.value.key);

  std::string out = e->independent;

  // Independent vowel accents also belong to the Vowel value.
  for (const Accent &a : v.value.accents) {
    const OtherEntry *accent = mapping_->other(a.key);

    if (accent) {
      out += accent->value;
    }
  }

  if (v.zwnj_after) {
    out += kZwnj;
  }

  if (v.zwj_after) {
    out += kZwj;
  }

  return out;
}

std::string Renderer::render(const Element &element) const {
  return std::visit(
      [this](const auto &e) -> std::string {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, Literal>) {
          return e.text;
        } else {
          return render(e);
        }
      },
      element);
}

std::string Renderer::render(const Document &doc) const {
  std::string out;

  for (const Element &e : doc) {
    out += render(e);
  }

  return out;
}

} // namespace okkhor::latin_to_bangla
// okkhor - interactive / piped Bangla phonetic transliteration.
//
//   okkhor                  read stdin line by line
//   okkhor amar sonar       Latin -> Bangla
//   okkhor --reverse আমি    Bangla -> Latin
//   okkhor --tokens kta     dump the internal representation
//
// Latin -> Bangla is the default direction.

#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "core/okkhor.hpp"

namespace {

void print_tokens(const std::vector<okkhor::Token> &tokens) {
  for (const okkhor::Token &token : tokens) {
    std::cout << " " << okkhor::token_type_name(token.type) << "("
              << token.canonical_key << " : " << token.value << ")";
  }

  std::cout << "\n";
}

void print_accents(const okkhor::Mapping &mapping,
                   const std::vector<okkhor::Accent> &accents) {
  for (const okkhor::Accent &accent : accents) {
    const auto *entry = mapping.other(accent.key);

    std::cout << " +accent(";

    if (entry) {
      std::cout << entry->canonical_key;
    } else {
      std::cout << "?";
    }

    std::cout << ")";
  }
}

void print_structure(const okkhor::Mapping &mapping,
                     const okkhor::Document &document) {
  std::cout << "structure   :";

  for (const okkhor::Element &element : document) {
    std::visit(
        [&](const auto &unit) {
          using T = std::decay_t<decltype(unit)>;

          if constexpr (std::is_same_v<T, okkhor::OrthographicUnit>) {
            std::cout << " [base=";

            // -------------------------------------------------
            // Base consonant
            // -------------------------------------------------

            if (const auto *bc =
                    std::get_if<okkhor::BaseConsonant>(&unit.base)) {

              const auto *consonant = mapping.consonant(bc->value.key);

              if (consonant) {
                std::cout << consonant->canonical_key;
              } else {
                std::cout << "?";
              }

              // Accents now belong to the consonant itself.
              print_accents(mapping, bc->value.accents);

            } else {
              std::cout << "vcons";
            }

            // -------------------------------------------------
            // Structural conjuncts
            // -------------------------------------------------

            for (const auto &dc : unit.conjuncts) {
              const auto *consonant = mapping.consonant(dc.value.key);

              std::cout << " +dcons(";

              if (consonant) {
                std::cout << consonant->canonical_key;
              } else {
                std::cout << "?";
              }

              // Accents belong to this conjunct.
              print_accents(mapping, dc.value.accents);

              std::cout << ")";
            }

            // -------------------------------------------------
            // Dependent vowel
            // -------------------------------------------------

            if (unit.vowel) {
              const auto *vowel = mapping.vowel(unit.vowel->value.key);

              std::cout << " +dvowel(";

              if (vowel) {
                std::cout << vowel->canonical_key;
              } else {
                std::cout << "?";
              }

              // Accents belong to the vowel.
              print_accents(mapping, unit.vowel->value.accents);

              std::cout << ")";
            }

            // -------------------------------------------------
            // Controls
            // -------------------------------------------------

            if (unit.explicit_hasanta) {
              std::cout << " +hasanta";
            }

            if (unit.zwnj_after) {
              std::cout << " +zwnj";
            }

            if (unit.zwj_after) {
              std::cout << " +zwj";
            }

            std::cout << "]";
          }

          // -----------------------------------------------------
          // Independent vowel
          // -----------------------------------------------------

          else if constexpr (std::is_same_v<T, okkhor::IndependentVowel>) {
            const auto *vowel = mapping.vowel(unit.value.key);

            std::cout << " [vowel(";

            if (vowel) {
              std::cout << vowel->canonical_key;
            } else {
              std::cout << "?";
            }

            // Accents belong to the Vowel.
            print_accents(mapping, unit.value.accents);

            std::cout << ")";

            if (unit.zwnj_after) {
              std::cout << " +zwnj";
            }

            if (unit.zwj_after) {
              std::cout << " +zwj";
            }

            std::cout << "]";
          }

          // -----------------------------------------------------
          // Literal
          // -----------------------------------------------------

          else if constexpr (std::is_same_v<T, okkhor::Literal>) {
            std::cout << " [literal \"" << unit.text << "\"]";
          }
        },
        element);
  }

  std::cout << "\n";
}

void describe(const okkhor::Engine &engine, const std::string &input,
              okkhor::WorkingDirection direction) {
  std::cout << "input       : " << input << "\n";

  // -----------------------------------------------------------------
  // Step 1: Tokenize
  // -----------------------------------------------------------------

  std::vector<okkhor::Token> raw_tokens =
      engine.tokenize_input(input, direction);

  std::cout << "raw tokens  :";
  print_tokens(raw_tokens);

  // -----------------------------------------------------------------
  // Latin -> Bangla
  // -----------------------------------------------------------------

  if (direction == okkhor::WorkingDirection::Forward) {

    // Step 2: Apply Latin -> Bangla rules.

    std::vector<okkhor::Token> rewritten_tokens =
        engine.latin_to_bangla_rules().apply(raw_tokens, engine.mapping());

    std::cout << "rule tokens :";
    print_tokens(rewritten_tokens);

    // Step 3: Parse into orthographic structure.

    const okkhor::Document document = engine.analyze_latin_to_bangla(input);

    print_structure(engine.mapping(), document);

    // Step 4: Render.

    std::cout << "output      : " << engine.transliterate_latin_to_bangla(input)
              << "\n";

    return;
  }

  // -----------------------------------------------------------------
  // Bangla -> Latin
  // -----------------------------------------------------------------

  // Step 2: Apply reversible Bangla -> Latin rules.

  // std::vector<okkhor::Token> rewritten_tokens =
  //     engine.bangla_to_latin_rules().apply(raw_tokens, engine.mapping());

  // print_tokens(rewritten_tokens);

  // Step 3: Parse into orthographic structure.

  const okkhor::Document document = engine.analyze_bangla_to_latin(input);

  print_structure(engine.mapping(), document);

  // Step 4: Render.

  std::string output = engine.transliterate_bangla_to_latin(input);

  std::cout << "output: " << output << "\n";
}

} // namespace

using namespace okkhor;

int main(int argc, char **argv) {
  std::string data_dir;
  bool verbose = false;

  WorkingDirection direction = WorkingDirection::Forward;

  std::vector<std::string> words;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--data" && i + 1 < argc) {
      data_dir = argv[++i];
    } else if (arg == "--tokens" || arg == "-v") {
      verbose = true;
    } else if (arg == "--reverse" || arg == "-r") {
      direction = WorkingDirection::Reverse;
    } else if (arg == "--help" || arg == "-h") {
      std::cout << "usage: okkhor "
                << "[--data DIR] "
                << "[--reverse] "
                << "[--tokens] "
                << "[text...]\n";

      std::cout << "\n"
                << "default: Latin -> Bangla\n"
                << "--reverse, -r: Bangla -> Latin\n"
                << "--tokens, -v: show internal representation\n"
                << "--data DIR: load external data from DIR\n";

      return 0;
    } else {
      words.push_back(arg);
    }
  }

  try {

    // No --data:
    //     use JSON data embedded in the executable.
    //
    // --data DIR:
    //     explicitly load JSON data from DIR.

    std::unique_ptr<okkhor::Engine> engine;

    if (data_dir.empty()) {
      engine = std::make_unique<okkhor::Engine>();
    } else {
      engine = std::make_unique<okkhor::Engine>(data_dir);
    }

    // -----------------------------------------------------------------
    // Arguments
    // -----------------------------------------------------------------

    if (!words.empty()) {
      std::string joined;

      for (std::size_t i = 0; i < words.size(); ++i) {

        if (i) {
          joined += ' ';
        }

        joined += words[i];
      }

      if (verbose) {
        describe(*engine, joined, direction);
      } else if (direction == WorkingDirection::Forward) {
        std::cout << engine->transliterate_latin_to_bangla(joined) << "\n";
      } else {
        std::cout << engine->transliterate_bangla_to_latin(joined) << "\n";
      }

      return 0;
    }

    // -----------------------------------------------------------------
    // stdin
    // -----------------------------------------------------------------

    std::string line;

    while (std::getline(std::cin, line)) {
      if (verbose) {
        describe(*engine, line, direction);
      } else if (direction == WorkingDirection::Forward) {
        std::cout << engine->transliterate_latin_to_bangla(line) << "\n";
      } else {
        std::cout << engine->transliterate_bangla_to_latin(line) << "\n";
      }
    }

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "okkhor: " << e.what() << "\n";

    return 1;
  }
}
#include "parser.hpp"

#include <iostream>
#include <optional>
#include <utility>

namespace okkhor::latin_to_bangla {

Document parse(
    const std::vector<Token>& tokens,
    const Mapping& mapping
) {
    Document doc;

    std::optional<OrthographicUnit> cur;

    // The unit currently being built.
    auto flush = [&] {
        if (cur) {
            doc.push_back(std::move(*cur));
            cur.reset();
        }
    };

    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const Token& t = tokens[i];

        switch (t.type) {

        case TokenType::Consonant: {
            // C + C -> BC + DC + VN
            // Otherwise start a fresh unit.

            Consonant c{t.canonical_key};

            if (cur && add_consonant(*cur, c))
                break;

            flush();

            cur = make_consonant(std::move(c));

            break;
        }

        case TokenType::VirtualConsonant: {
            // ` always opens a new unit with a
            // visually-nothing base.

            flush();

            cur = make_virtual_consonant();

            break;
        }

        case TokenType::Vowel: {
            // C + V -> BC + DV
            // A vowel with no open base stands alone.

            Vowel v{t.canonical_key};

            if (cur && add_vowel(*cur, v))
                break;

            flush();

            IndependentVowel independent{
                std::move(v),
                false,
                false
            };

            doc.push_back(
                std::move(independent)
            );

            break;
        }

        case TokenType::Accent: {
            Accent a{t.canonical_key};

            // First try to attach the accent to the
            // currently active orthographic unit.
            if (cur && add_accent(*cur, a))
                break;

            // Otherwise, if the previous document element
            // is an independent vowel, attach the accent
            // to its Vowel value.
            if (
                !cur &&
                !doc.empty() &&
                std::holds_alternative<IndependentVowel>(
                    doc.back()
                )
            ) {
                auto& independent =
                    std::get<IndependentVowel>(
                        doc.back()
                    );

                independent.value.accents.push_back(
                    std::move(a)
                );

                break;
            }

            // No existing character can own this accent.
            // Preserve it as an orthographic unit with
            // a virtual base.
            flush();

            OrthographicUnit u =
                make_virtual_consonant();

            add_accent(u, std::move(a));

            doc.push_back(
                std::move(u)
            );

            break;
        }

        case TokenType::Hasanta: {
            const bool next_is_zwnj =
                i + 1 < tokens.size() &&
                tokens[i + 1].type == TokenType::ZWNJ;

            if (cur) {
                add_hasanta(*cur);

                if (next_is_zwnj) {
                    add_zwnj(*cur);

                    ++i;
                    // Consume the ZWNJ together with Hasanta.
                }

                flush();

                break;
            }

            OrthographicUnit u =
                make_virtual_consonant();

            add_hasanta(u);

            if (next_is_zwnj) {
                add_zwnj(u);
                ++i;
            }

            doc.push_back(
                std::move(u)
            );

            break;
        }

        case TokenType::ZWNJ: {
            if (cur) {
                add_zwnj(*cur);
                flush();
            }
            else if (
                !doc.empty() &&
                std::holds_alternative<IndependentVowel>(
                    doc.back()
                )
            ) {
                std::get<IndependentVowel>(
                    doc.back()
                ).zwnj_after = true;
            }
            else {

           const OtherEntry* e =mapping.other(t.canonical_key);



                doc.push_back(
                    Literal{e->value}
                );
            }

            break;
        }

        case TokenType::ZWJ: {
            if (cur) {
                add_zwj(*cur);
                flush();
            }
            else if (
                !doc.empty() &&
                std::holds_alternative<IndependentVowel>(
                    doc.back()
                )
            ) {
                std::get<IndependentVowel>(
                    doc.back()
                ).zwj_after = true;
            }
            else {
                doc.push_back(
                    Literal{t.canonical_key}
                );
            }

            break;
        }

        case TokenType::Punctuation: {
            flush();

            const OtherEntry* e =
                mapping.other(t.canonical_key);

            if (!e) {
                throw std::runtime_error(
                    "Unknown punctuation(" +
                    t.canonical_key +
                    ")"
                );
            }

            doc.push_back(
                Literal{e->value}
            );

            break;
        }

        case TokenType::Whitespace:
        case TokenType::Unknown:
        default: {
            // Whitespace and unknown input close
            // the current unit and pass through untouched.

            flush();

            doc.push_back(
                Literal{t.value}
            );

            break;
        }
        }
    }

    flush();

    return doc;
}

} // namespace okkhor::latin_to_bangla
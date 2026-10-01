#pragma once

#include <map>
#include <string>
#include <vector>

#include "core/mapping.hpp"
#include "core/tokenizer.hpp"
#include "util/json.hpp"


namespace okkhor::latin_to_bangla {

struct Condition {
    bool always = false;

    bool word_start = false;

    bool after_consonant = false;
    bool after_vowel = false;

    bool before_consonant = false;
    bool before_vowel = false;
};

struct Action {
    enum class Type {
        TokenList,
        Literal
    };

    Type type;

    std::vector<std::string> token_values;
    std::string literal_value;
};

struct ContextRule {
    Condition condition;
    Action action;
};

class RuleEngine {
public:
    RuleEngine() = default;

    void load(
        const json::Value& doc,
        Mapping& mapping
    );

    std::vector<Token> apply(
        const std::vector<Token>& tokens,
        const Mapping& mapping
    ) const;

private:
    std::map<std::string, std::vector<ContextRule>> rules_;

    bool evaluate_condition(
        const Condition& cond,
        bool is_word_start,
        TokenType last_type,
        TokenType next_type
    ) const;
};

} // namespace okkhor::latin_to_bangla
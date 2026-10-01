#include "rules.hpp"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "core/tokenizer.hpp"
#include "util/json.hpp"

namespace okkhor::bangla_to_latin {
namespace {

/*
 * Convert a rule action into the literal string that the
 * renderer is expected to produce.
 *
 * Example:
 *
 * ["j", "NG"]
 *
 * becomes:
 *
 * "jNG"
 *
 * The action is interpreted using the FORWARD mapping because
 * rule actions describe the canonical Latin/Bangla semantic
 * representation used by the normal forward mapping.
 */
std::string parse_token_action(const json::Value &value,
                               const Mapping &mapping) {
  if (!value.is_array()) {
    throw std::runtime_error("rule token value must be an array");
  }

  std::string result;
  for (const json::Value &item : value.as_array()) {
    if (!item.is_string()) {
      throw std::runtime_error("rule token value must contain only strings");
    }

    const std::string &key = item.as_string();

    /*
     * Make sure every action item is a valid forward
     * mapping token.
     *
     * We don't need the resulting Token anymore.
     * The rule engine operates on the rendered string.
     */
    const okkhor::Rule *rule = mapping.lookup(key, Direction::Forward);
    if (!rule) {
      throw std::runtime_error("rule action contains unknown forward token: " +
                               key);
    }

    result += key;
  }

  return result;
}

/*
 * Literal actions are already strings.
 */
std::string parse_literal_action(const json::Value &value) {
  if (value.is_string())
    return value.as_string();

  if (value.is_array()) {
    std::string result;
    for (const json::Value &item : value.as_array()) {
      if (!item.is_string()) {
        throw std::runtime_error("literal value must contain only strings");
      }
      result += item.as_string();
    }
    return result;
  }

  throw std::runtime_error(
      "literal value must be a string or an array of strings");
}

/*
 * Determine whether a rule is unconditional.
 *
 * This is deliberately separate from reversibility.
 */
bool is_always_rule(const json::Value &when) {
  if (!when.is_object())
    return false;

  const json::Value *always = when.find("always");
  return always && always->is_bool() && always->as_bool();
}

/*
 * Reversibility is explicitly opt-in.
 */
bool is_reversible_rule(const json::Value &condition) {
  const json::Value *reversible = condition.find("reversible");
  return reversible && reversible->is_bool() && reversible->as_bool();
}

} // namespace

void RuleEngine::load(const json::Value &doc, const Mapping &mapping) {
  const json::Value *rules = doc.find("rules");
  if (!rules || !rules->is_object()) {
    throw std::runtime_error("Bangla -> Latin: expected 'rules' object");
  }

  rules_.clear();

  /*
   * Every JSON rule is originally a FORWARD rule:
   *
   * key -> action
   *
   * For the BN -> LT engine we only construct an inverse
   * when the rule explicitly declares itself reversible
   * and is unconditional.
   *
   * Example:
   *
   * "gg" -> ["j", "NG"]
   *
   * becomes:
   *
   * "jNG" -> "gg"
   */
  for (const auto &[key, value] : rules->as_object()) {
    if (!value.is_array()) {
      throw std::runtime_error("Bangla -> Latin rule must be an array: " + key);
    }

    for (const json::Value &condition : value.as_array()) {
      if (!condition.is_object()) {
        throw std::runtime_error(
            "Bangla -> Latin rule condition must be an object: " + key);
      }

      const json::Value *when = condition.find("when");
      const json::Value *action = condition.find("action");

      if (!when || !action) {
        throw std::runtime_error(
            "Bangla -> Latin rule requires 'when' and 'action': " + key);
      }

      /*
       * Context-dependent rules cannot safely be inverted
       * into a simple post-render string replacement.
       *
       * Example:
       *
       * ng -> Ng
       *
       * where the actual behavior depends on context.
       */
      if (!is_always_rule(*when))
        continue;

      /*
       * Explicit opt-in for reverse transformation.
       *
       * This prevents:
       *
       * ggg -> gg
       *
       * from becoming:
       *
       * gg -> ggg
       */
      if (!is_reversible_rule(condition))
        continue;

      if (!action->is_object()) {
        throw std::runtime_error("Bangla -> Latin action must be an object: " +
                                 key);
      }

      const json::Value *type = action->find("type");
      const json::Value *action_value = action->find("value");

      if (!type || !type->is_string()) {
        throw std::runtime_error(
            "Bangla -> Latin action requires string 'type': " + key);
      }

      if (!action_value) {
        throw std::runtime_error("Bangla -> Latin action requires 'value': " +
                                 key);
      }

      const std::string &action_type = type->as_string();

      /*
       * -------------------------------------------------
       * TOKEN ACTION
       * -------------------------------------------------
       *
       * Example:
       *
       * "gg" -> ["j", "NG"]
       *
       * becomes:
       *
       * "jNG" -> "gg"
       *
       * The rule engine works on the rendered string,
       * so no Special token is created.
       */
      if (action_type == "token") {
        const std::string pattern = parse_token_action(*action_value, mapping);

        if (pattern.empty()) {
          throw std::runtime_error(
              "Bangla -> Latin reversible rule produced an empty pattern: " +
              key);
        }

        bool rmap = condition.bool_or("rmap", false);

        Rule rule;
        rule.pattern = !rmap ? pattern : key;
        rule.replacement = !rmap ? key : pattern;
        rules_.push_back(std::move(rule));
        continue;
      }

      /*
       * -------------------------------------------------
       * LITERAL ACTION
       * -------------------------------------------------
       *
       * A literal action is not safely invertible.
       *
       * Example:
       *
       * w -> "ও"
       *
       * There is no general way to know that "ও" originally
       * came from "w".
       */
      if (action_type == "literal") {

        const std::string pattern = parse_literal_action(*action_value);

        if (pattern.empty()) {
          throw std::runtime_error(
              "Bangla -> Latin reversible rule produced an empty pattern: " +
              key);
        }

        bool rmap = condition.bool_or("rmap", false);

        Rule rule;
        rule.pattern = !rmap ? pattern : key;
        rule.replacement = !rmap ? key : pattern;
        rules_.push_back(std::move(rule));
        continue;
      }

      throw std::runtime_error("Bangla -> Latin unsupported action type: " +
                               action_type);
    }
  }
}

/*
 * Apply post-render rules.
 *
 * The input is the fully rendered Latin string.
 *
 * No Token objects are involved here.
 *
 * Therefore Special tokens cannot reach the parser, renderer,
 * or this stage.
 */
std::string RuleEngine::apply(const std::string &input) const {
  std::string current = input;

  /*
   * Rules are applied sequentially so that transformations
   * can compose.
   */
  for (const Rule &rule : rules_) {
    if (rule.pattern.empty())
      continue;

    std::string output;
    std::size_t position = 0;

    while (position < current.size()) {
      if (position + rule.pattern.size() <= current.size() &&
          current.compare(position, rule.pattern.size(), rule.pattern) == 0) {
        output += rule.replacement;
        position += rule.pattern.size();
        continue;
      }

      output.push_back(current[position]);
      ++position;
    }

    current = std::move(output);
  }

  return current;
}

} // namespace okkhor::bangla_to_latin
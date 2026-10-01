#pragma once

#include <string>
#include <vector>

#include "core/mapping.hpp"
#include "util/json.hpp"

namespace okkhor::bangla_to_latin {

struct Rule {
  std::string pattern;
  std::string replacement;
};

class RuleEngine {
public:
  void load(const json::Value &doc, const Mapping &mapping);
  std::string apply(const std::string &input) const;

private:
  std::vector<Rule> rules_;
};

} // namespace okkhor::bangla_to_latin
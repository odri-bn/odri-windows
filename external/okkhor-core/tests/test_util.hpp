#pragma once
#include <iostream>
#include <string>

namespace testing {

inline int failures = 0;
inline int checks = 0;

inline void check(const std::string &label, const std::string &got,
                  const std::string &want) {
  ++checks;
  if (got == want)
    return;
  ++failures;
  std::cout << "FAIL " << label << "\n  expected: \"" << want
            << "\"\n  actual  : \"" << got << "\"\n";
}

inline void check_true(const std::string &label, bool value) {
  ++checks;
  if (value)
    return;
  ++failures;
  std::cout << "FAIL " << label << " (expected true)\n";
}

inline int report(const char *suite) {
  std::cout << suite << ": " << (checks - failures) << "/" << checks
            << " checks passed\n";
  return failures == 0 ? 0 : 1;
}

} // namespace testing

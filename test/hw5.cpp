#include <fstream>
#include <sstream>

#include "Automaton.hpp"
#include "gtest/gtest.h"
#include "regex.hpp"

using namespace formal;

namespace {
void checkResult(const std::string& regex, const std::string& pattern,
                 int expected) {
  auto dfa = formal::regexToDFA(regex);
  ASSERT_EQ(dfa.prefixEqual(pattern), expected);
}
}  // namespace

TEST(HW5, example1) { checkResult("aab+*a..", "ababab", 5); }

TEST(HW5, example2) { checkResult("acb..bab.c.*.ab.ba.+.+*a.", "cb", 0); }

TEST(HW5, example3) { checkResult("a1b+c..", "ac", 2); }

TEST(HW5, wrongChars) {
  ASSERT_THROW(formal::regexToDFA("fasjhfhjak....,897931<d"),
               std::invalid_argument);
}

TEST(HW5, wrongRegex) {
  ASSERT_THROW(formal::regexToDFA("abc."),
               std::invalid_argument);
}

TEST(HW5, ratata) {
  std::string pattern = "ratatatatatatatatatatata";
  checkResult("rat+*.a.", pattern, pattern.size());
}

TEST(HW5, onechar_closure) { checkResult("x*", "xxxxxxxd", 7); }

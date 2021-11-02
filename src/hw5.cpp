#include <iostream>
#include <fstream>
#include "Automaton.hpp"
#include "regex.hpp"

int main() {
  std::string regex;
  std::string pattern;
  std::cin >> regex >> pattern;

  auto dfa = formal::regexToDFA(regex);

  std::cout << dfa.prefixEqual(pattern) << std::endl;

  std::ofstream out("out.gv");
  dfa.dump(out);
  return 0;
}

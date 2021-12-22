#include <iostream>
#include <fstream>
#include "Automaton.hpp"

int main() {
  formal::Automaton automaton = formal::readAutomaton(std::cin);

  automaton.eNFAtoDFA();

  std::ofstream out("out.gv");
  automaton.dump(out);
  return 0;
}

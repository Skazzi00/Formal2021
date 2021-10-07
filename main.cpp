#include <iostream>
#include <fstream>
#include "Automaton.hpp"

int main() {
  std::ifstream in("../example1.in");
  formal::Automaton automaton = formal::readAutomaton(in);
  std::ofstream out("out.gv");
  automaton.eNFAToNFA();
  automaton.NFAToDFA();
  automaton.dump(out);
  return 0;
}

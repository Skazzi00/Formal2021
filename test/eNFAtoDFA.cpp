#include <fstream>
#include <sstream>
#include "Automaton.hpp"
#include "gtest/gtest.h"

using namespace formal;

void checkEqual(std::istream &os1, std::istream &os2) {
  size_t termCnt1, termCnt2;
  os1 >> termCnt1;
  os2 >> termCnt2;
  ASSERT_EQ(termCnt1, termCnt2);
  for (size_t i = 0; i < termCnt1; i++) {
    int v1, v2;
    os1 >> v1;
    os2 >> v2;
    ASSERT_EQ(v1, v2);
  }

  int s1, s2;
  os1 >> s1;
  os2 >> s2;
  ASSERT_EQ(s1, s2);

  size_t statesCnt1, statesCnt2;
  os1 >> statesCnt1;
  os2 >> statesCnt2;
  ASSERT_EQ(statesCnt1, statesCnt2);

  size_t edgeCnt1, edgeCnt2;
  os1 >> edgeCnt1;
  os2 >> edgeCnt2;
  ASSERT_EQ(edgeCnt1, edgeCnt2);

  for (size_t i = 0; i < edgeCnt1; i++) {
    int from1, from2;
    int to1, to2;
    char ch1, ch2;

    os1 >> from1 >> to1 >> ch1;
    os2 >> from2 >> to2 >> ch2;
    ASSERT_EQ(from1, from2);
    ASSERT_EQ(to1, to2);
    ASSERT_EQ(ch1, ch2);
  }

}

void test_eNFAtoDFA(const std::string &inPath, const std::string &nfaPath, const std::string &dfaPath) {
  std::ifstream in(inPath);
  std::ofstream nullStream;
  formal::Automaton automaton = formal::readAutomaton(in, nullStream);

  automaton.eNFAToNFA();
  std::stringstream ssNFA;
  automaton.dumpText(ssNFA);
  std::ifstream nfaStream(nfaPath);
  checkEqual(ssNFA, nfaStream);

  automaton.NFAToDFA();
  std::stringstream ssDFA;
  automaton.dumpText(ssDFA);
  std::ifstream dfaStream(dfaPath);
  checkEqual(ssDFA, dfaStream);
}


TEST(AutomatonCorrectness, example1) {
  test_eNFAtoDFA("NFA_test/example1.in", "NFA_test/example1NFA.in", "NFA_test/example1DFA.in");
}
TEST(AutomatonCorrectness, example2) {
  test_eNFAtoDFA("NFA_test/example2.in", "NFA_test/example2NFA.in", "NFA_test/example2DFA.in");
}
TEST(AutomatonCorrectness, example3) {
  test_eNFAtoDFA("NFA_test/example3.in", "NFA_test/example3NFA.in", "NFA_test/example3DFA.in");
}
TEST(AutomatonCorrectness, example4) {
  test_eNFAtoDFA("NFA_test/example4.in", "NFA_test/example4NFA.in", "NFA_test/example4DFA.in");
}
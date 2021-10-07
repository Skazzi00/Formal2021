#pragma once
#include <vector>
#include <set>
#include <map>
#include <queue>

namespace formal {

constexpr char EPS = '$';

struct Edge {
  int from;
  int to;
  char c;
};

class Automaton {
  using TransitionsType = std::map<std::pair<int, char>, std::set<int>>;
  size_t statesCnt;
  int start;
  TransitionsType transition;
  std::set<int> term;
 public:
  Automaton(size_t statesCnt, int start, const std::vector<Edge> &edges, const std::vector<int> &term)
      : statesCnt(statesCnt), start(start), term(term.begin(), term.end()) {
    for (const auto&[from, to, c]: edges) {
      transition[{from, c}].insert(to);
    }
  }

 private:
  void dfs(std::vector<bool> &used, int v) {
    used[v] = true;
    for (const auto &to: transition[{v, EPS}]) {
      if (used[to]) continue;
      dfs(used, to);
    }
  }

  void epsClosure() {
    std::vector<bool> used;
    for (int i = 0; i < statesCnt; i++) {
      used.assign(statesCnt, false);
      dfs(used, i);
      for (int to = 0; to < statesCnt; ++to) {
        if (i == to) continue;
        if (used[to]) transition[{i, EPS}].insert(to);
      }
    }
  }

  void addEpsTerm() {
    for (int i = 0; i < statesCnt; i++) {
      if (term.contains(i)) continue;
      for (int to: transition[{i, EPS}]) {
        if (term.contains(to)) {
          term.insert(i);
          break;
        }
      }
    }
  }

  void addEpsTrans() {
    for (int i = 0; i < statesCnt; i++)
      for (int to: transition[{i, EPS}])
        for (char c = 'a'; c <= 'z'; c++)
          transition[{i, c}].insert(transition[{to, c}].begin(), transition[{to, c}].end());
  }

  void deleteEps() {
    for (int i = 0; i < statesCnt; i++)
      transition.erase({i, EPS});
  }

 public:
  void eNFAToNFA() {
    epsClosure();
    addEpsTerm();
    addEpsTrans();
    deleteEps();
  }

  void NFAToDFA() {
    std::map<std::set<int>, int> id;
    int curId = 0;

    auto getIndex = [&id, &curId](const std::set<int> &st) {
      if (id.contains(st)) return id[st];
      id[st] = curId++;
      return id[st];
    };

    TransitionsType dfaTrans;
    std::set<int> newStates;
    std::set<int> newTerms;
    std::queue<std::set<int>> p;
    p.push({start});
    newStates.insert(getIndex({start}));
    while (!p.empty()) {
      std::set<int> curState = p.front();
      int cur = getIndex(curState);
      for (const auto &v: curState) {
        if (term.contains(v)) newTerms.insert(cur);
      }
      p.pop();
      for (char c = 'a'; c <= 'z'; c++) {
        std::set<int> newState;
        for (const auto &v: curState) {
          newState.insert(transition[{v, c}].begin(), transition[{v, c}].end());
        }
        // devil state
        if (!newState.empty())
          dfaTrans[{cur, c}] = {getIndex(newState)};
        if (!newStates.contains(getIndex(newState))) {
          p.push(newState);
          newStates.insert(getIndex(newState));
        }
      }
    }
    start = getIndex({start});
    statesCnt = newStates.size();
    term = std::move(newTerms);
    transition = std::move(dfaTrans);
  }

  void dump(std::ostream &out) {
    out << "digraph finite_state_machine {\n"
           "rankdir=LR;\n"
           "size=\"8,5\"\n"
           "n0 [label= \"\", shape=none,height=.0,width=.0]\n";
    out << "node [shape = doublecircle]; ";
    for (int v: term) {
      out << v << ' ';
    }
    out << ";\n";
    out << "node [shape = circle];\n";
    out << "n0 -> " << start << '\n';
    for (const auto &[key, toSet] : transition) {
      for (const auto &to : toSet) {
        int from = key.first;
        char c = key.second;
        out << from << "->" << to << " [label = \"" << c << "\"];\n";
      }
    }
    out << "}";
  }

};

Automaton readAutomaton(std::istream &in = std::cin, std::ostream &out = std::cout) {
  size_t nStates;
  out << "Enter number of states: ";
  in >> nStates;
  int start;
  out << "Enter start state: ";
  in >> start;
  size_t nEdges;
  out << "Enter number of transitions: ";
  in >> nEdges;
  out << "Enter edges (<from> <to> <a..z|$>):\n";
  std::vector<Edge> edges;
  edges.reserve(nEdges);
  for (size_t i = 0; i < nEdges; i++) {
    int from, to;
    char ch;
    in >> from >> to >> ch;
    edges.push_back({from, to, ch});
  }
  std::vector<int> term;
  size_t nTerm;
  out << "Enter number of term states: ";
  in >> nTerm;
  out << "Enter term states: ";
  for (size_t i = 0; i < nTerm; i++) {
    int t;
    in >> t;
    term.push_back(t);
  }
  return Automaton(nStates, start, edges, term);
}
}

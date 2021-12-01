#pragma once
#include <iostream>
#include <map>
#include <ostream>
#include <queue>
#include <set>
#include <vector>

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
  Automaton(size_t statesCnt, int start, const std::vector<Edge> &edges,
            const std::vector<int> &term)
      : statesCnt(statesCnt), start(start), term(term.begin(), term.end()) {
    for (const auto &[from, to, ch] : edges) {
      transition[{from, ch}].insert(to);
    }
  }

 private:
  std::pair<std::set<int>, std::set<int>> split(const std::set<int> &r, const std::set<int> &c, char a) {
    std::pair<std::set<int>, std::set<int>> res;
    for (int st : r) {
      if (c.contains(*transition[{st, a}].begin())) {
        res.first.insert(st);
      } else {
        res.second.insert(st);
      }
    }
    return res;
  }

 public:
  void minimizeDFA() {
    std::set<std::set<int>> p;
    p.insert(term);
    std::set<int> nonTerm;
    for (int i = 0; i < statesCnt; i++) {
      if (!term.contains(i)) nonTerm.insert(i);
    }
    p.insert(nonTerm);

    std::queue<std::pair<std::set<int>, char>> s;
    for (char c = 'a'; c <= 'z'; c++) {
      s.push({term, c});
      s.push({nonTerm, c});
    }
    while (!s.empty()) {
      auto[C, a] = s.front();
      s.pop();
      for (auto r = p.begin(); r != p.end();) {
        auto[r1, r2] = split(*r, C, a);
        if (!r1.empty() && !r2.empty()) {
          auto rCopy = r++;
          p.erase(rCopy);
          p.insert(r1);
          p.insert(r2);
          for (char c = 'a'; c <= 'z'; c++) {
            s.push({r1, c});
            s.push({r2, c});
          }
        }
      }
    }

    std::map<int, int> clazz_id;
    int index = 0;
    for (const auto &clazz : p) {
      for (int st : clazz) {
        clazz_id[st] = index;
      }
      index++;
    }
    TransitionsType transition_new;
    std::set<int> term_new;
    index = 0;
    for (const auto &clazz : p) {
      for (char c = 'a'; c <= 'z'; c++) {
        transition_new[{index, c}] = {clazz_id[*transition[{*clazz.begin(), c}].begin()]};
      }
      if (term.contains(*clazz.begin())) {
        term_new.insert(index);
      }
      if (clazz.contains(start)){
        start = index;
      }
      index++;
    }
    statesCnt = index;
    term = std::move(term_new);
    transition = std::move(transition_new);
  }

  void fullDfa() {
    bool devil = false;
    for (int state = 0; state < statesCnt; state++) {
      for (char c = 'a'; c <= 'z'; c++) {
        if (transition[{state, c}].empty()) {
          devil = true;
          transition[{state, c}].insert(statesCnt);
        }
      }
    }
    if (devil) {
      for (char c = 'a'; c <= 'z'; c++) transition[{statesCnt, c}].insert(statesCnt);
      statesCnt++;
    }
  }

 private:
  void findClosure(std::vector<bool> &used, int v) {
    used[v] = true;
    for (const auto &to : transition[{v, EPS}]) {
      if (used[to]) continue;
      findClosure(used, to);
    }
  }

  void epsClosure() {
    std::vector<bool> used;
    for (int i = 0; i < statesCnt; i++) {
      used.assign(statesCnt, false);
      findClosure(used, i);
      for (int to = 0; to < statesCnt; ++to) {
        if (i == to) continue;
        if (used[to]) transition[{i, EPS}].insert(to);
      }
    }
  }

  void addEpsTerm() {
    for (int i = 0; i < statesCnt; i++) {
      if (term.contains(i)) continue;
      for (int to : transition[{i, EPS}]) {
        if (term.contains(to)) {
          term.insert(i);
          break;
        }
      }
    }
  }

  void addEpsTrans() {
    for (int i = 0; i < statesCnt; i++)
      for (int to : transition[{i, EPS}])
        for (char ch = 'a'; ch <= 'z'; ch++)
          transition[{i, ch}].insert(transition[{to, ch}].begin(),
                                     transition[{to, ch}].end());
  }

  void deleteEps() {
    for (int i = 0; i < statesCnt; i++) transition.erase({i, EPS});
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
    std::queue<std::set<int>> statesQueue;

    statesQueue.push({start});
    newStates.insert(getIndex({start}));
    while (!statesQueue.empty()) {
      std::set<int> curState = statesQueue.front();
      statesQueue.pop();

      for (char c = 'a'; c <= 'z'; c++) {
        std::set<int> newState;
        for (const auto &v : curState) {
          newState.insert(transition[{v, c}].begin(), transition[{v, c}].end());
        }

        // devil state
        if (newState.empty()) {
          continue;
        }

        dfaTrans[{getIndex(curState), c}] = {getIndex(newState)};
        if (!newStates.contains(getIndex(newState))) {
          statesQueue.push(newState);
          newStates.insert(getIndex(newState));
        }
      }

      for (const auto &v : curState) {
        if (term.contains(v)) newTerms.insert(getIndex(curState));
      }
    }

    start = getIndex({start});
    statesCnt = newStates.size();
    term = std::move(newTerms);
    transition = std::move(dfaTrans);
  }

  void eNFAtoDFA() {
    eNFAToNFA();
    NFAToDFA();
  }

  int prefixEqual(const std::string &str) {
    int curState = start;
    int ans = 0;
    for (int i = 0; i < str.size(); i++) {
      if (!transition.contains({curState, str[i]})) return ans;

      curState = *transition[{curState, str[i]}].begin();

      if (term.contains(curState)) ans = i + 1;
    }

    return ans;
  }

  void dump(std::ostream &out) {
    out << "digraph finite_state_machine {\n"
           "rankdir=LR;\n"
           "size=\"8,5\"\n"
           "n0 [label= \"\", shape=none,height=.0,width=.0]\n";
    out << "node [shape = doublecircle]; ";
    for (int v : term) {
      out << v << ' ';
    }
    out << ";\n";
    out << "node [shape = circle];\n";
    out << "n0 -> " << start << '\n';
    for (const auto &[key, toSet] : transition) {
      for (const auto &to : toSet) {
        int from = key.first;
        char ch = key.second;
        out << from << "->" << to << " [label = \"" << ch << "\"];\n";
      }
    }
    out << "}";
  }

  void dumpText(std::ostream &out) {
    out << term.size();
    out << '\n';
    for (int v : term) {
      out << v << ' ';
    }
    out << '\n';
    out << start << '\n';
    out << statesCnt << '\n';
    size_t edgeCnt = 0;
    for (const auto &[key, toSet] : transition) {
      edgeCnt += toSet.size();
    }
    out << edgeCnt << '\n';
    for (const auto &[key, toSet] : transition) {
      for (const auto &to : toSet) {
        int from = key.first;
        char ch = key.second;
        out << from << " " << to << " " << ch << '\n';
      }
    }
  }
};

inline Automaton readAutomaton(std::istream &in = std::cin,
                               std::ostream &out = std::cout) {
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
}  // namespace formal

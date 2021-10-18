#pragma once

#include <stack>
#include <stdexcept>
#include <vector>

#include "Automaton.hpp"
#include "common.hpp"

namespace formal {

Automaton regexToDFA(const std::string& regex) {
  struct State {
    std::vector<Edge> edges;
    int term;
    int start;
    char startCh;
  };
  int index = 0;

  std::stack<State> stack;

  for (auto c : regex) {
    if (isAlpha(c)) {
      stack.push({{}, index, index, c});
      index++;
      continue;
    } else if (c == '1') {
      stack.push({{}, index, index, '$'});
      index++;
      continue;
    }
    switch (c) {
      case '+': {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        int leftConnect = index++;
        int rightConnect = index++;

        std::vector<Edge> edges;
        edges.insert(edges.end(), left.edges.begin(), left.edges.end());
        edges.insert(edges.end(), right.edges.begin(), right.edges.end());

        
        edges.push_back({leftConnect, left.start, left.startCh});
        edges.push_back({leftConnect, right.start, right.startCh});
        edges.push_back({left.term, rightConnect, '$'});
        edges.push_back({right.term, rightConnect, '$'});

        stack.push({std::move(edges), rightConnect, leftConnect, '$'});
      } break;

      case '.': {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        std::vector<Edge> edges;
        edges.insert(edges.end(), left.edges.begin(), left.edges.end());
        edges.insert(edges.end(), right.edges.begin(), right.edges.end());

        edges.push_back({left.term, right.start, right.startCh});

        stack.push({std::move(edges), right.term, left.start, left.startCh});
      } break;

      case '*': {
        auto top = stack.top();
        stack.pop();

        top.edges.push_back(
            {top.term, top.start, top.start == top.term ? top.startCh : '$'});

        stack.push(top);
      } break;
      default:
        throw std::invalid_argument("Invalid regex string: Unknown char");
    }
  }

  if (stack.size() != 1) {
    throw std::invalid_argument(
        "Invalid regex string: Stack size != 1 after parse");
  }

  auto& finalState = stack.top();

  int finalStart = index++;

  finalState.edges.push_back(
      {finalStart, finalState.start, finalState.startCh});




  Automaton res(index, finalStart, finalState.edges, {finalState.term});
  res.eNFAtoDFA();
  return res;
}

}  // namespace formal
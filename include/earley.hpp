#pragma once
#include <utility>
#include <vector>
#include <set>
#include <string>

namespace formal::earley {

struct Rule {
    char from;
    std::string to;
    Rule(char from, std::string to) : from(from), to(std::move(to)) {}
    bool operator<(const Rule &rhs) const {
        if (from < rhs.from)
            return true;
        if (rhs.from < from)
            return false;
        return to < rhs.to;
    }
    bool operator>(const Rule &rhs) const {
        return rhs < *this;
    }
    bool operator<=(const Rule &rhs) const {
        return !(rhs < *this);
    }
    bool operator>=(const Rule &rhs) const {
        return !(*this < rhs);
    }
};

struct Situation {
    Rule rule;
    int dot;
    int pos;
    Situation(Rule rule, int dot, int pos) : rule(std::move(rule)), dot(dot), pos(pos) {}

    int getNext() const {
        return dot == rule.to.size() ? -1 : rule.to[dot];
    }

    bool operator<(const Situation &rhs) const {
        if (rule < rhs.rule)
            return true;
        if (rhs.rule < rule)
            return false;
        if (dot < rhs.dot)
            return true;
        if (rhs.dot < dot)
            return false;
        return pos < rhs.pos;
    }
    bool operator>(const Situation &rhs) const {
        return rhs < *this;
    }
    bool operator<=(const Situation &rhs) const {
        return !(rhs < *this);
    }
    bool operator>=(const Situation &rhs) const {
        return !(*this < rhs);
    }
};

bool isTerm(char c) {
    return ('a' <= c && c <= 'z') || c == '(' || c == ')';
}

bool isNTerm(char c) {
    return !isTerm(c);
}

void scan(std::vector<std::set<Situation>> &d, int j, const std::vector<Rule> &grammar, const std::string &w) {
    if (j == 0) {
        return;
    }

    for (const auto &situation : d[j - 1]) {
        int c = situation.getNext();
        if (c == -1) continue;
        if (isNTerm(c)) continue;
        if (c == w[j - 1]) {
            d[j].emplace(situation.rule, situation.dot + 1, situation.pos);
        }
    }
}

void complete(std::vector<std::set<Situation>> &d, int j, const std::vector<Rule> &grammar, const std::string &w) {
    for (const auto &B : d[j]) {
        if (B.getNext() != -1) continue;
        for (const auto &A : d[B.pos]) {
            if (A.getNext() != B.rule.from) continue;
            d[j].emplace(A.rule, A.dot + 1, A.pos);
        }
    }
}

void predict(std::vector<std::set<Situation>> &d, int j, const std::vector<Rule> &grammar, const std::string &w) {
    for (const auto &B : d[j]) {
        if (B.getNext() == -1 || isTerm(B.getNext())) continue;
        for (auto &rule: grammar) {
            if (rule.from == B.getNext()) {
                d[j].emplace(rule, 0, j);
            }
        }
    }
}

bool earley(const std::vector<Rule> &grammar, const std::string &w) {
    size_t n = w.size();
    std::vector<std::set<Situation>> d(n + 1);
    d[0].emplace(Rule{'$', "S"}, 0, 0);
    for (int j = 0; j <= n; j++) { // <
        scan(d, j, grammar, w);
        size_t prevSize = d[j].size();
        do {
            prevSize = d[j].size();
            complete(d, j, grammar, w);
            predict(d, j, grammar, w);
        } while (prevSize != d[j].size());
    }
    return d[n].contains(Situation(Rule('$', "S"), 1, 0));
}

} // namespace formal::earley
import unittest
from functools import lru_cache
from typing import Set, Dict, FrozenSet, Tuple, Callable, Union
from common_lr import *


@dataclass(frozen=True)
class Item:
    rule: Rule
    pos: int
    next: str

    def __str__(self):
        return '[' + self.rule.lhs + "->" + self.rule.rhs[0:self.pos] + '.' + self.rule.rhs[
                                                                              self.pos:] + ', ' + self.next + ']'

    def get_next(self):
        if self.pos == len(self.rule.rhs):
            return None
        return self.rule.rhs[self.pos]

    def can_reduce(self):
        return self.pos == len(self.rule.rhs)


@lru_cache
def is_eps(nonterm: str, grammar: FrozenSet[Rule]) -> bool:
    for rule in grammar:
        if rule.lhs != nonterm:
            continue
        for c in terminals:
            if c in rule.rhs:
                continue
        flag = True
        for N in rule.rhs:
            flag &= is_eps(N, grammar)
        if flag:
            return True
    return False


def get_first_nonterm(nonterm: str, grammar: FrozenSet[Rule]) -> FrozenSet[str]:
    res = set()
    for rule in grammar:
        if rule.lhs != nonterm:
            continue
        if len(rule.rhs) == 0:
            continue

        for c_n in rule.lhs:
            if c_n in terminals:
                res.add(c_n)
                break
            add_set = get_first_nonterm(c_n, grammar)
            for c in add_set:
                res.add(c)
            if not is_eps(c_n, grammar):
                break
    return frozenset(res)


def get_first(first: str, grammar: FrozenSet[Rule]) -> FrozenSet[str]:
    res = set()
    for index in range(len(first)):
        if first[index] in terminals:
            res.add(first[index])
            return frozenset(res)
        else:
            res.add(get_first_nonterm(first[index], grammar))
            if not is_eps(first[index], grammar):
                return frozenset(res)
    return frozenset(res)


def closure(items: Set[Item], grammar: FrozenSet[Rule]) -> FrozenSet[Item]:
    res = items.copy()
    for item in items:
        if not is_nonterm(item.get_next()):
            return frozenset(res)
        flag = True
        while flag:
            flag = False
            new = set()
            for cur_item in res:
                for rule in set(filter(lambda x: x.lhs == cur_item.get_next(), grammar)):
                    for next_ch in get_first(cur_item.rule.rhs[cur_item.pos + 1:] + cur_item.next, grammar):
                        if Item(rule, 0, next_ch) in res:
                            continue
                        new.add(Item(rule, 0, next_ch))
                        flag = True
            for cur in new:
                res.add(cur)

    return frozenset(res)


def get_dfa(grammar: FrozenSet[Rule]) -> Tuple[Dict[FrozenSet[Item], int], Set[Edge]]:
    states = dict()
    edges = set()
    index = 0
    q = [closure({Item(Rule('$', 'S#'), 0, '#')}, grammar)]
    states[q[0]] = index
    index += 1
    while len(q) > 0:
        cls = q.pop(0)
        new_sets = dict()
        for item in cls:
            c = item.get_next()
            if c is None:
                continue
            if c not in new_sets.keys():
                new_sets[c] = set()
            new_sets[c].add(Item(item.rule, item.pos + 1, item.next))

        for c, items in new_sets.items():
            n_cls = closure(items, grammar)
            if n_cls not in states.keys():
                states[n_cls] = index
                index += 1
                q.append(n_cls)
            edges.add(Edge(states[cls], states[n_cls], c))

    return states, edges


def build_control_table(grammar: FrozenSet[Rule]):
    states, edges = get_dfa(grammar)
    # for i in list(map(lambda it: (str(list(map(str, it[0]))), it[1]), states.items())):
    #     print(i)
    # print(list(map(str, edges)))
    table = dict()
    for edge in edges:
        table[edge.start, edge.char] = TableElem(id=edge.end,
                                                 action=ActionType.GOTO if is_nonterm(edge.char) else ActionType.SHIFT)
    for state, index in states.items():
        for item in state:
            if item.can_reduce():
                for c in terminals:
                    if (index, c) in table.keys():
                        continue  # maybe conflict
                    table[index, c] = TableElem(action=ActionType.REDUCE, rule=item.rule)

    return table


class Tokenizer:
    def __init__(self, w: str):
        self.index = 0
        self.w = w

    def get_functions(self):
        def cur():
            return self.w[self.index] if self.index < len(self.w) else '#'

        def next_token():
            self.index += 1

        return cur, next_token


def get_parser(grammar: FrozenSet[Rule]) -> Callable[[str], Union[bool]]:
    table = build_control_table(grammar)

    def parser(w: str) -> bool:
        stack = [0]
        cur_token, next_token = Tokenizer(w).get_functions()

        def top():
            return stack[-1]

        def push(item):
            stack.append(item)

        def pop():
            return stack.pop()

        while len(stack) > 0:
            cur_state = top()
            elem = table.get((cur_state, cur_token()), None)
            if elem is None:
                return False
            if elem.action == ActionType.REDUCE:
                if elem.rule.lhs == '$':
                    return True
                for _ in elem.rule.rhs:
                    pop()
                    pop()
                s = top()
                push(elem.rule.lhs)
                goto = table.get((s, elem.rule.lhs), None)
                if goto is None or goto.action != ActionType.GOTO:
                    return False
                push(goto.id)
            elif elem.action == ActionType.SHIFT:
                push(cur_token())
                push(elem.id)
                next_token()

        raise RuntimeError("How did you get there?")

    return parser


class TestLR1(unittest.TestCase):
    def test_ab_abc(self):
        g = {Rule('S', 'aB'), Rule('B', 'b'), Rule('B', 'bc')}
        parser = get_parser(frozenset(g))

        self.assertTrue(parser('ab'))
        self.assertTrue(parser('abc'))

        self.assertFalse(parser('a'))
        self.assertFalse(parser('acb'))
        self.assertFalse(parser('ac'))

    def test_expr(self):
        g = {Rule('S', 'A'), Rule('A', 'A+F'), Rule('A', 'F'), Rule('F', 'F*T'), Rule('F', 'T'), Rule('T', '(A)'),
             Rule('T', 'x'), Rule('T', '1'), Rule('T', '0'), }
        parser = get_parser(frozenset(g))

        for s in ['x', 'x+x', 'x*x', '(x)+x*x*(x+x+x*x+x)', '(x+1)*x+(1*(1+x)+x)+0']:
            self.assertTrue(parser(s))

        for s in ['x+', '+x', 'x1', 'x+(x*x', '1+1+x**x']:
            self.assertFalse(parser(s))


if __name__ == '__main__':
    unittest.main()

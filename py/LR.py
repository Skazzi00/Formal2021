import unittest
from typing import Set, Dict, FrozenSet, Tuple, Callable, Union
from common_lr import *
from dataclasses import dataclass


@dataclass(frozen=True)
class Item:
    rule: Rule
    pos: int

    def __str__(self):
        return self.rule.lhs + "->" + self.rule.rhs[0:self.pos] + '.' + self.rule.rhs[self.pos:]

    def get_next(self):
        if self.pos == len(self.rule.rhs):
            return None
        return self.rule.rhs[self.pos]

    def can_reduce(self):
        return self.pos == len(self.rule.rhs)


def closure(item: Item, grammar: Set[Rule]) -> FrozenSet[Item]:
    res = {item}
    if not is_nonterm(item.get_next()):
        return frozenset(res)
    flag = True
    while flag:
        flag = False
        new = set()
        for cur_item in res:
            for rule in set(filter(lambda x: x.lhs == cur_item.get_next(), grammar)):
                if Item(rule, 0) in res:
                    continue
                new.add(Item(rule, 0))
                flag = True
        for cur in new:
            res.add(cur)

    return frozenset(res)


def get_dfa(grammar: Set[Rule]) -> Tuple[Dict[FrozenSet[Item], int], Set[Edge]]:
    states = dict()
    edges = set()
    index = 0
    q = [closure(Item(Rule('$', 'S#'), 0), grammar)]
    states[q[0]] = index
    index += 1
    while len(q) > 0:
        cls = q.pop(0)
        for item in cls:
            c = item.get_next()
            if c is None:
                continue
            n_cls = closure(Item(item.rule, item.pos + 1), grammar)
            if n_cls not in states.keys():
                states[n_cls] = index
                index += 1
                q.append(n_cls)
            edges.add(Edge(states[cls], states[n_cls], c))

    return states, edges


class ActionType(Enum):
    SHIFT = 1
    REDUCE = 2
    GOTO = 3


def build_control_table(grammar: Set[Rule]):
    states, edges = get_dfa(grammar)
    table = dict()
    for edge in edges:
        table[edge.start, edge.char] = TableElem(id=edge.end,
                                                 action=ActionType.GOTO if is_nonterm(edge.char) else ActionType.SHIFT)
    for state, index in states.items():
        for item in state:
            if item.can_reduce():
                for c in terminals:
                    if (index, c) in table.keys():
                        raise RuntimeError("Not LR(0) grammar")
                    table[index, c] = TableElem(action=ActionType.REDUCE, rule=item.rule)
    # for i in list(map(lambda it: (str(list(map(str, it[0]))), it[1]), states.items())):
    #     print(i)
    # print(list(map(str, edges)))
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


def get_parser(grammar: Set[Rule]) -> Callable[[str], Union[bool, None]]:
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

            if elem.action == ActionType.SHIFT:
                push(cur_token())
                push(elem.id)
                next_token()
            elif elem.action == ActionType.REDUCE:
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

        raise RuntimeError("How did you get there?")

    return parser


class TestLR0(unittest.TestCase):
    def test_aabcc(self):
        g = {Rule('S', 'aSc'), Rule('S', 'b')}
        parser = get_parser(g)

        self.assertTrue(parser("b"))
        self.assertTrue(parser("aabcc"))
        self.assertTrue(parser("abc"))
        self.assertTrue(parser("aaaaaabcccccc"))

        self.assertFalse(parser("aacc"))
        self.assertFalse(parser("aabc"))
        self.assertFalse(parser("aaabcccc"))
        self.assertFalse(parser("aaaaab"))

    def test_ab(self):
        g = {Rule('S', 'AA'), Rule('A', 'aA'), Rule('A', 'b')}
        parser = get_parser(g)

        self.assertTrue(parser("aaaaabaab"))
        self.assertTrue(parser("aaaaabb"))
        self.assertTrue(parser("baaaaaab"))
        self.assertTrue(parser("bb"))

        self.assertFalse(parser("abbb"))
        self.assertFalse(parser("abbaaaaab"))
        self.assertFalse(parser("aaaaab"))
        self.assertFalse(parser("b"))


if __name__ == '__main__':
    unittest.main()

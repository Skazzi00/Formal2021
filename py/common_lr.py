from dataclasses import dataclass
from enum import Enum
from string import ascii_lowercase


@dataclass(frozen=True)
class Rule:
    lhs: str
    rhs: str

    def __str__(self):
        return self.lhs + "->" + self.rhs


@dataclass(frozen=True)
class Edge:
    start: int
    end: int
    char: str

    def __str__(self):
        return str(self.start) + '--(' + self.char + ')>' + str(self.end)


def is_nonterm(c: str) -> bool:
    return c is not None and c.isupper() or c == '$'


terminals = ascii_lowercase + '(+-*)#01'


class ActionType(Enum):
    SHIFT = 1
    REDUCE = 2
    GOTO = 3


@dataclass(frozen=True)
class TableElem:
    action: ActionType
    id: int = None
    rule: Rule = None

    def __str__(self):
        return str(self.action) + ' : ' + str(self.rule) if self.action == ActionType.REDUCE else str(
            self.action) + ' : ' + str(self.id)

################################################################################
# This file is part of Configurable Simulation Framework (CSF) (formerly SAP)
# Copyright (c) 2017 by Yifei Zheng
# Unauthorized copy, distribution and modification of this file is prohibited.
#
# This file contains structures used to store data from source code, and useful
# processing routines for code generation.
################################################################################
from . import expr
from collections import OrderedDict as odict
from itertools import chain, combinations
import enum
from . import resolver


class FreeVar:
    def __init__(self, name, type, initializer=None):
        self.name = name
        self.type = type
        self.value = initializer


OP_TO_INST = {
    '+': 'ADD',
    '-': 'SUB',
    '*': 'MUL',
    '/': 'DIV'
}

OP_WRITER = {
}


def chunker(seq, size):
    return (seq[pos:pos + size] for pos in range(0, len(seq), size))


def get_n_in_both(seq1, seq2, n) -> list:
    """
    Get first :param n: elements appeared both in seq1 and seq2,
    return all if there's less than n elements, iterate on seq1.
    """
    it = (i for i in seq1 if i in seq2)
    ret = []
    for i in range(n):
        ret.append(next(it))
    return ret


def get_n_idx_in_both(seq1, seq2, n) -> list:
    """
    Get first :param n: indices of elements appeared both in
    seq1 and seq2, return all if there's less than n elements.
    Iterate on seq1.
    """
    it = (i for i, j in enumerate(seq1) if j in seq2)
    ret = []
    for i in range(n):
        ret.append(next(it))
    return ret


def findDerivatives(expr):
    def customorder(expression, DIFF=0):
        if isinstance(expression, tuple):
            if expression[0] == 'diff':
                DIFF += 1
            for i in expression[1]:
                yield from customorder(i, DIFF)
            if expression[0] == 'diff':
                DIFF -= 1
        elif isinstance(expression, str):
            yield resolver.cNVar(expression, DIFF)

    yield from customorder(expr)


def listVars(expr):
    def noorder(expression):
        if isinstance(expression, tuple):
            for i in expression[1]:
                yield from noorder(i)
        elif isinstance(expression, str):
            yield expression

    yield from noorder(expr)


class Rule:
    def __init__(self, content, attribute):
        self.content = expr.stringToExpr(content[0]), expr.stringToExpr(content[1])
        self.attribute = attribute

    @staticmethod
    def writeExpr(expression: tuple):
        for i in expr.postorder(expression):
            if isinstance(i, expr.Op):
                if i.name in OP_TO_INST:
                    yield f'{OP_TO_INST[i.name]}\n'
                else:
                    yield f'CALL\t{i.name}\n'
            else:
                if isinstance(i, str):
                    if '$' in i:
                        name = i[1:].split('.', 1)[0]
                        try:
                            num = int(name)
                            yield f'ITER\t{num}\n'
                        except ValueError:
                            pass
                    i = f'VAR [{i}]'
                yield f'PUSHI\t{i}\n'

    def write(self):
        yield from Rule.writeExpr(self.content[0])
        yield from Rule.writeExpr(self.content[1])
        yield 'SUB\n'
        yield f'SOVS\t{self.solveFor}\n'

    def __repr__(self):
        return f'<{self.attribute}> {self.content[0]} = {self.content[1]}'


class EqnType(enum.IntEnum):
    ALG_S = 1
    ALG_M = 2
    DIFF_S = 3


class RulePack:
    """
    This is the class representing packed rule on highest level.
    That is, usually the pack may be divided further for actual
    computation, as ODE, algebraic equation system, etc.
    Thus, instances of this class are created purely by inspecting variables.
    """

    def __init__(self, content):
        self.pack = content
        self.solveFor = None
        self.order = None

    def extend(self, iterable):
        self.pack.extend(iterable)

    def resolve(self, known):
        Known = set(known)  # cannot rebind known, we're modifying as ref
        for i in self.pack:
            i.minus(Known)

        self.order = resolver.resolve(self.pack)

        def cat(t):
            return t.name + str(t.order)

        with open('a.dot', 'w') as f:
            f.write('digraph g {')
            for num, step in enumerate(self.order):
                if isinstance(step[0], list):
                    pass
                elif isinstance(step[0], int):
                    for i in self.pack[step[0]].diffs:
                        if cat(i) != cat(step[1]):
                            f.write(f'{cat(i)} -> {cat(step[1])} [label = {num}];')
                else:
                    f.write(f'{cat(step[0])} -> {cat(step[1])} [label = {num}];')
            f.write('{rank=same; x0, y0, z0}')
            f.write('{rank=same; x1, y1, z1}')
            f.write('{rank=same; x2, y2, z2}')
            f.write('}')

        print(self.order)

    def write(self):
        for i in self.order:
            pass

    def __len__(self):
        return len(self.pack)

    def __iter__(self):
        yield from self.pack

    def __repr__(self):
        return '{\n\t' + '\n\t'.join(repr(i) for i in self.pack) + '\n} -> ' + repr(self.solveFor)


class CondRule:
    """
    None condition is used for else
    Empty content is used for assertion
    """

    def __init__(self, content):
        self.crpack = content
        self.vars = set()
        for cond, rulepack in chunker(self.crpack, 2):
            for rule in rulepack:
                self.vars |= rule.vars

    COMP = ['==', '=', '!=', '<>', '<=', '<', '>=', '>']  # ORDER MATTERS, PREFER LONGER

    @staticmethod
    def writeCond(condition):
        for i in CondRule._infixCondToPostfix(condition):
            if isinstance(i, tuple):
                yield f'{i[0]}\n'
            else:
                comp = next(x for x in CondRule.COMP if x in i)
                lhs, rhs = i.split(comp)
                yield from Rule.writeExpr(expr.stringToExpr(lhs))
                yield from Rule.writeExpr(expr.stringToExpr(rhs))

    @staticmethod
    def _infixCondToPostfix(condition):
        if isinstance(condition, tuple):
            if len(condition) is 2:
                yield condition[1]
                yield (condition[0],)  # protect operator with tuple
            elif len(condition) is 3:
                yield from CondRule._infixCondToPostfix(condition[0])
                yield from CondRule._infixCondToPostfix(condition[2])
                yield (condition[1],)
        else:
            yield condition

    @staticmethod
    def writeRulePack(rulepack):
        for rule in rulepack:
            yield from rule.write()

    def write(self):
        yield 'ADIF\n'
        yield from self.writeCond(self.crpack[0])
        yield from self.writeRulePack(self.crpack[1])
        if not self.crpack[-2]:
            for i, j in chunker(self.crpack[2:-2], 2):
                yield 'ADEIF\n'
                yield from self.writeCond(i)
                yield from self.writeRulePack(j)
            yield 'ADEL\n'
            yield from self.writeCond(self.crpack[-2])
            yield from self.writeRulePack(self.crpack[-1])
        else:
            for i, j in chunker(self.crpack[2:], 2):
                yield 'ADEIF\n'
                yield from self.writeCond(i)
                yield from self.writeRulePack(j)

    def __repr__(self):
        return repr(self.crpack)


class RuleResolvingStruct:
    def __init__(self, idx, rule):
        self.idx = idx
        self.vars = set(chain(listVars(rule[0]), listVars(rule[1])))
        self.diffs = set(chain(findDerivatives(rule[0]), findDerivatives(rule[1])))

    def minus(self, set):
        self.vars = {i for i in self.vars if i not in set}
        self.diffs = {i for i in self.diffs if i.name not in set}

    def __repr__(self):
        return repr(self.diffs)


class Space:
    def __init__(self):
        self.watched = []
        self.vars = {}
        self.constinfo = {'RF': FreeVar('RF', 'double')}
        self.funcs = {}  # reserved
        self.rules = []

    def addRule(self, content, attrib):
        self.rules.append(Rule(content, attrib))

    def addCondRule(self, cond, content, attrib):
        self.rules.append(CondRule((cond, content, attrib)))

    def process(self):
        """
        Determines the resolving order, remove potential duplicates of watched variables
        """
        print("Processing...")
        self.watched = self.removeDup(self.watched)
        known = self.watched[:] + ['t']
        known.extend(self.constinfo)
        rrs = [RuleResolvingStruct(idx, i.content) for idx, i in enumerate(self.rules)]
        # must loop by reference
        idx = 0
        while idx < len(rrs):
            i = rrs[idx]
            candidates = i.vars.difference(known)
            if len(candidates) == 1:
                cand = candidates.pop()
                i.solveFor = cand
                known.append(cand)
                idx += 1
            elif not candidates:
                cand_idx = next(idx for idx, x in enumerate(known) if x in i.vars)
                i.solveFor = known[cand_idx]
                known.append(known.pop(cand_idx))  # move cand to last
                idx += 1
            else:  # request more and solve together
                i = self._get_enough(rrs, idx, known, candidates)
                i.resolve(known)
                idx += 1
        self.rules = rrs

    @staticmethod
    def _get_enough(rrs, idx, known, candidates):
        rrs[idx] = RulePack(rrs[idx:idx + len(candidates)])
        del rrs[idx + 1:idx + len(candidates)]
        i = rrs[idx]  # rebind i
        if len(candidates) == len(i):
            i.solveFor = candidates
        elif len(candidates) < len(i):
            cand_idx = get_n_idx_in_both(known, i.vars, len(i) - len(candidates))
            i.solveFor = known[cand_idx]
        else:
            return Space._get_enough(rrs, idx, known, candidates)
        return i

    @staticmethod
    def removeDup(seq):
        return list(odict.fromkeys(seq).keys())

    def write(self):
        self.process()
        print('Generating bytecode...', end=' ')
        tmp = []
        tmp.append('.def; definition of object\n')
        for i in self.watched:
            if i in self.varinfo:
                tmp.append(f'VEC\t{i}\n')
            else:
                tmp.append(f'{i}\n')
        tmp.append('.init:; initialize objects\n')
        counter = 0
        for i, V in self.varinfo.items():
            if V.Type == 'object':
                counter += 1
                tmp.append(f'OBJECT\t{i}\n')
            for lhs, rhs in V.value:
                tmp.append(f'SET\t{lhs},{rhs}\n')
            tmp.append(f'END\n')

        yield f'NUM\t{counter}\n'
        yield from tmp
        yield f'PUSH\t{self.varinfo["time"].value}\n'
        yield f'PUSH\t{self.varinfo["step"].value}\n'
        yield '.rule:; rule definition start\n'
        for i in self.rules:
            yield from i.write()
        print('Done')

    def __repr__(self):
        return f'watched: {self.watched}\nvars: {self.vars}\nrules:\n' + '\n'.join(repr(i) for i in self.rules)

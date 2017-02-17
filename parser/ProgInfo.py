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
from itertools import chain

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


class FreeVar:
    def __init__(self, name, Type, initializer=None):
        self.name = name
        self.Type = Type
        self.value = initializer

    def __repr__(self):
        return f'Name {self.name}, value {self.value}'


class NVar(FreeVar):
    """
    This class is derived from variable, and provides storage
    for order of derivative, which is required to resolve the
    solving order.
    """

    def __init__(self, name, Type='double', diff=0, initializer=None):
        super().__init__(name, Type, initializer)
        self.diff = diff

    def __repr__(self):
        return self.name + "'" * self.diff + f': {self.name}, order {self.diff}'


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
            yield NVar(expression, diff=DIFF)

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
        self.vars = set(chain(listVars(self.content[0]), listVars(self.content[1])))

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


class RulePack:
    def __init__(self, content):
        self.pack = content


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


class Space:
    def __init__(self):
        self.watched = ['t']
        self.vars = {}
        self.constinfo = {'RF': FreeVar('RF', 'double')}
        self.funcs = {}  # reserved
        self.rules = []

    def addRule(self, content, attrib):
        self.rules.append(Rule(content, attrib))

    def addCondRule(self, cond, content, attrib):
        self.rules.append(CondRule((cond, content, attrib)))

    class RuleResolvingStruct:
        def __init__(self, idx, vars, diffs):
            self.idx = idx
            self.vars = vars
            self.diffs = diffs

    @staticmethod
    def resolve(rule, known: list):
        diff = rule.vars.difference(known)
        if len(diff) is 1:
            known.append(rule.solveFor)
            return diff.pop()
        elif not diff:
            idx = next(idx for idx, x in enumerate(known) if x in rule.vars)
            known.append(known.pop(idx))  # only pop it to the last
            return known[-1]
        return len(diff)  # returns how many further equations to request

    def process(self):
        """
        Determines the resolving order, remove potential duplicates of watched variables
        """
        print("Processing...")
        self.watched = self.removeDup(self.watched)
        known = self.watched[:]
        known.extend(self.constinfo)

        for i in self.rules:
            solveFor = self.resolve(i, known)
            if isinstance(solveFor, str):
                pass
            else:
                raise Exception("Unimplemented")

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

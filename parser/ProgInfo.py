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
from . import resolver


class FreeVar:
    def __init__(self, name, type, initializer=None):
        self.name = name
        self.type = type
        self.value = initializer


OP_TO_INST = expr.operators


def chunker(seq, size):
    return (seq[pos:pos + size] for pos in range(0, len(seq), size))


def removeDup(seq):
    return list(odict.fromkeys(seq).keys())


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
    ITER = 1

    def __init__(self, content, attribute):
        self.content = expr.stringToExpr(content[0]), expr.stringToExpr(content[1])
        self.attribute = attribute

    @staticmethod
    def writeExpr(expression: tuple):
        if isinstance(expression, str):
            var = expression
            if '$' in var:
                name, var = var[1:].split('.', 1)
                try:
                    Rule.ITER = max(Rule.ITER, int(name))
                except ValueError:
                    pass
            return var
        elif isinstance(expression, int):
            return str(expression)
        if len(expression) > 1:
            return OP_TO_INST[expression[0]].write(expression[1], Rule.writeExpr)
        else:
            raise ValueError()

    def write(self):
        yield 'return math::op::sub('
        yield self.writeExpr(self.content[0])
        yield ',\n'
        yield self.writeExpr(self.content[1])
        yield ');\n'

    def __repr__(self):
        return f'<{self.attribute}> {self.content[0]} = {self.content[1]}'


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

    def write(self):
        def cat(t):
            return f'{t.name}_{t.order}_'

        for num, step in enumerate(self.order):
            if isinstance(step[0], list):
                yield f'SVA\t{self.pack[step[0]]}\n'
            elif isinstance(step[0], int):
                yield '{0} = math::solver::solve_algebraic_single([](double {0}){{\n'.format(cat(step[1]))
                yield from self.pack[step[0]].rule.write()
                yield '}}, {});\n'.format(cat(step[1]))
            else:
                if step[1].order > step[0].order:
                    yield f'{cat(step[1])} = ({cat(step[0])}n - {cat(step[0])})/step;\n'
                else:
                    yield f'math::solver::integrate_update({cat(step[0])},{cat(step[1])});\n'

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
        yield 'if ('
        yield from self.writeCond(self.crpack[0])
        yield ') {\n'
        yield from self.writeRulePack(self.crpack[1])
        yield '}\n'
        if not self.crpack[-2]:
            for i, j in chunker(self.crpack[2:-2], 2):
                yield 'ADEIF\n'
                yield from self.writeCond(i)
                yield from self.writeRulePack(j)
            yield 'else {\n'
            yield from self.writeCond(self.crpack[-2])
            yield from self.writeRulePack(self.crpack[-1])
        else:
            for i, j in chunker(self.crpack[2:], 2):
                yield 'else if('
                yield from self.writeCond(i)
                yield from self.writeRulePack(j)

    def __repr__(self):
        return repr(self.crpack)


class RuleResolvingStruct:
    def __init__(self, idx, rule):
        self.idx = idx
        self.rule = rule
        self.vars = set(chain(listVars(rule.content[0]), listVars(rule.content[1])))
        self.diffs = set(chain(findDerivatives(rule.content[0]), findDerivatives(rule.content[1])))

    def minus(self, set):
        self.vars = {i for i in self.vars if i not in set}
        self.diffs = {i for i in self.diffs if i.name not in set}

    def resolve(self, known):
        self.minus(set(known))
        self.order = removeDup(resolver.resolve([self]))
        print(self.order)

    def write(self):
        def cat(t):
            return f'{t.name}_{t.order}_'

        for num, step in enumerate(self.order):
            if isinstance(step[0], list):
                yield f'SVA\t{self.rule}\n'
            elif isinstance(step[0], int):
                yield '{0}n = math::solver::solve_algebraic_single([](double {0}){{\n'.format(cat(step[1]))
                yield from self.rule.write()
                yield '}}, {});\n'.format(cat(step[1]))
            else:
                if step[1].order > step[0].order:
                    yield f'{cat(step[1])} = ({cat(step[0])}n - {cat(step[0])})/step;\n'
                else:
                    yield f'math::solver::integrate_update({cat(step[0])},{cat(step[1])});\n'

    def __repr__(self):
        return repr(self.rule)


class Space:
    def __init__(self):
        self.watched = []
        self.objects = odict()
        self.vars = {"t": FreeVar('t', 'double', 1), "step": FreeVar('step', 'double', 1e-3)}
        self.constinfo = {'RF': FreeVar('RF', 'double')}
        self.funcs = {}  # reserved
        self.rules = []

    def addWatch(self, name, type='double'):
        self.watched.append(FreeVar(name, type))

    def addObj(self, name, init=[]):
        self.objects[name] = FreeVar(name, 'Object', init)

    def addRule(self, content, attrib):
        self.rules.append(Rule(content, attrib))

    def addCondRule(self, cond, content, attrib):
        self.rules.append(CondRule((cond, content, attrib)))

    def process(self):
        """
        Determines the resolving order, remove potential duplicates of watched variables
        """
        print("Processing...")
        self.watched = removeDup(self.watched)
        known = self.watched[:] + ['t']
        known.extend(self.constinfo)
        rrs = [RuleResolvingStruct(idx, i) for idx, i in enumerate(self.rules)]
        # must loop by reference
        idx = 0
        while idx < len(rrs):
            i = rrs[idx]
            candidates = i.vars.difference(known)
            if len(candidates) == 1:
                i.resolve(known)
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

    def write(self):
        print('Generating code...', end=' ')
        yield '#include "csf_includes.hpp"\n'
        yield 'struct Object {\n'
        for var in self.watched:
            yield f'{var.type} {var.name};\n'
        yield '};\n'
        yield '\n'
        yield f'Object storage[{len(self.objects)}]'' = {\n'
        for V in self.objects.values():
            yield '{'
            for lhs, rhs in V.value:
                yield rhs.write()
            yield '},\n'
        yield '};\n'
        yield '\n'
        yield 'struct shared_ {\n'
        yield 'enum object_name_ {\n'
        yield ', '.join(str(i) for i in self.objects)
        yield '};\n'
        yield 'template <object_name_ i>\n'
        yield 'Object& get(){\n'
        yield 'return storage[i];\n'
        yield '}\n'
        yield '};\n'
        yield '\n'
        yield 'int main() {\n'
        yield 'shared_ srd_;'
        for i in self.vars.values():
            if i.type == 'vector':
                yield f'vector {i.name} = {i.value};\n'
            else:
                yield f'double {i.name} = {i.value};\n'
        yield 'double time_ = 0;\n'
        tmp = [f'while (time_ < {self.vars["t"].value})''{\n',
               'for (comb.reset(); comb.get(0); ++comb){\n']
        for i in self.rules:
            tmp.extend(i.write())
        tmp.insert(0, f'combination<{Rule.ITER}> comb(0, {len(self.objects)});\n')
        yield from tmp
        yield '}\n'
        yield 'time_ += step;\n'
        yield '}\n'
        yield '}\n'
        print('Done')

    def __repr__(self):
        return f'watched: {self.watched}\nvars: {self.vars}\nrules:\n' + '\n'.join(repr(i) for i in self.rules)

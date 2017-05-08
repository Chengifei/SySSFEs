################################################################################
# This file is part of ATOM
# Copyright (c) 2017 by Yifei Zheng
# Unauthorized copy, distribution and modification of this file is prohibited.
#
# This file contains structures used to store data from source code, and useful
# processing routines for code generation.
################################################################################
import sys, os.path
from . import templating

sys.path.insert(0, os.path.abspath('..'))
from parser import expr
from collections import OrderedDict as odict
from itertools import chain, combinations
from . import resolver


class FreeVar:
    def __init__(self, name, type, initializer=None):
        self.name = name
        self.type = type
        self.value = initializer


OP_TO_INST = expr.operators


def union_all(iterable):
    a = set()
    for i in iterable:
        a.update(i)
    return a


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


class SolvingStep:
    def __init__(self, content):
        self.content = content
        if isinstance(self.content[0], list):
            self.type = 0
        elif isinstance(self.content[0], int):
            self.type = 1
        else:
            self.type = 2

    def write(self, pack):
        def cat(t):
            return f'{t.name}_{t.order}_'

        if self.type == 0:
            return f'math::solver::solve_algebraic_sys{pack[self.content[0]]}\n'
        elif self.type == 1:
            return '{0} = math::solver::solve_algebraic_single([](double {0}){{\n'.format(
                cat(self.content[1])) + pack[
                       self.content[0]].write() + f'}}, {cat(self.content[1])});\n'
        else:
            if self.content[1].order > self.content[0].order:
                return f'{cat(self.content[1])} = ({cat(self.content[0])}n - {cat(self.content[0])})/step;\n'
            else:
                return f'math::solver::integrate_update({cat(self.content[0])},{cat(self.content[1])});\n'


class TrackedSolvingStep(SolvingStep):
    def __init__(self, content):
        super().__init__(content)
        self.use = None
        self.update = None


def processSteps(rules, steps):
    tracked_steps = []
    for i in steps:
        this = TrackedSolvingStep(i)
        if self.type == 0:
            self.use
            self.update
        elif self.type == 1:
            self.use = rules[self.content[1]].vars
            self.update = self.content[1]
        else:
            self.use = self.content[0]
            self.update = self.content[1]
    

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
        return 'return math::op::sub(' + self.writeExpr(self.content[0]) + ',\n' + self.writeExpr(
            self.content[1]) + ');\n'

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
        self.all_forms_dict = {}
        for rule in self.pack:
            for var in rule.diffs:
                self.all_forms_dict.setdefault(var.name, []).append(var.order)
        self.solveFor = None
        self.order = None

    def extend(self, iterable):
        self.pack.extend(iterable)

    def resolve(self, known):
        known = set(known)
        for i in self.pack:
            i.minus(known)

        self.order = resolver.resolve(self.pack, self.all_forms_dict)
        return union_all(i.vars for i in self.pack)

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
        for cond, rulepack in self.crpack:
            for rule in rulepack:
                self.vars |= rule.vars

    COMP = ['==', '=', '!=', '<>', '<=', '<', '>=', '>']  # ORDER MATTERS, PREFER LONGER

    @staticmethod
    def writeCond(condition):
        for i in CondRule._infixCondToPrefix(condition):
            if isinstance(i, tuple):
                yield f'{i[0]}\n'
            else:
                comp = next(x for x in CondRule.COMP if x in i)
                lhs, rhs = i.split(comp)
                yield from Rule.writeExpr(expr.stringToExpr(lhs))
                yield from Rule.writeExpr(expr.stringToExpr(rhs))

    @staticmethod
    def _infixCondToPrefix(condition):
        if isinstance(condition, tuple):
            if len(condition) is 2:
                yield (condition[0], 1)  # protect operator with tuple
                yield condition[1]
            elif len(condition) is 3:
                yield (condition[1], 2)
                yield from CondRule._infixCondToPrefix(condition[0])
                yield from CondRule._infixCondToPrefix(condition[2])
        else:
            yield condition

    @staticmethod
    def writeRules(rulepack):
        for rule in rulepack:
            yield from rule.write()

    def write(self):
        yield 'if ('
        yield from self.writeCond(self.crpack[0])
        yield ') {\n'
        yield from self.writeRules(self.crpack[1])
        yield '}\n'
        if not self.crpack[-2]:
            for i, j in self.crpack[1:-1]:
                yield 'else if (\n'
                yield from self.writeCond(i)
                yield from self.writeRules(j)
            yield 'else {\n'
            yield from self.writeCond(self.crpack[-2])
            yield from self.writeRules(self.crpack[-1])
        else:
            for i, j in self.crpack[1:]:
                yield 'else if('
                yield from self.writeCond(i)
                yield from self.writeRules(j)

    def __repr__(self):
        return repr(self.crpack)


class RuleResolvingStruct:
    def __init__(self, idx, rule):
        self.idx = idx
        self.rule = rule
        self.vars = set(chain(listVars(rule.content[0]), listVars(rule.content[1])))
        self.diffs = set(chain(findDerivatives(rule.content[0]), findDerivatives(rule.content[1])))
        self.all_forms_dict = None  # uninitialized here, must do immediately before resolve

    def minus(self, set):
        self.vars = {i for i in self.vars if i not in set}
        self.diffs = {i for i in self.diffs if i.name not in set}

    def resolve(self, known):
        self.minus(set(known))
        self.all_forms_dict = {}
        for var in self.diffs:
            self.all_forms_dict.setdefault(var.name, []).append(var.order)
        self.order = removeDup(resolver.resolve([self], self.all_forms_dict))
        return self.vars

    def __repr__(self):
        return repr(self.rule)


class Space:
    def __init__(self):
        self.watched = odict()
        self.objects = odict()
        self.vars = {"t": FreeVar('t', 'double', 1), "step": FreeVar('step', 'double', 1e-3)}
        self.funcs = {}  # reserved
        self.rules = []
        self.steps = None

    def addWatch(self, name, type='double'):
        self.watched[name] = FreeVar(name, type)

    def addObj(self, name, init=[]):
        self.objects[name] = FreeVar(name, 'Object', init)

    def addRule(self, content, attrib):
        self.rules.append(Rule(content, attrib))

    def addCondRule(self, cond, content, attrib):
        self.rules.append(CondRule((cond, content, attrib)))

    def process(self):
        print("Processing...")
        known = list(chain(iter(self.watched), iter(self.vars)))
        rrs = [RuleResolvingStruct(idx, i) for idx, i in enumerate(self.rules)]

        # must loop by reference
        idx = 0
        while idx < len(rrs):
            Idx = idx
            solveFor = 0
            needMore = True
            while needMore:
                candidates = rrs[Idx].vars.difference(known)
                Idx += 1
                solveFor += len(candidates)
                needMore = (Idx - idx) > solveFor
            rrs[idx] = RulePack(rrs[idx:Idx])
            del rrs[idx + 1:Idx]
            known.extend(rrs[idx].resolve(known))
            idx = Idx

        self.steps = [step for pack in rrs for step in pack.order]

    def write(self):
        def object_init(obj_vals):
            for V in obj_vals:
                yield '{'
                for lhs, rhs in V.value:
                    yield rhs.write()
                yield '},\n'
        def global_vars(var_vals, iterlen, object_len):
            for i in var_vals:
                yield f'{i.type} {i.name} = {i.value};'
            yield f'combination<{iterlen}> comb(0, {object_len});'
        print('Generating code...')
        gen = templating.template('codegen/template')
        while True:
            n = next(gen)
            if n is not None:
                yield n
            else:
                break
        yield gen.send(f'{var.type} {var.name};' for var in self.watched.values())
        while True:
            n = next(gen)
            if n is not None:
                yield n
            else:
                break
        yield gen.send([str(len(self.objects))])  # wrap as an iterable
        while True:
            n = next(gen)
            if n is not None:
                yield n
            else:
                break
        yield gen.send(object_init(self.objects.values()))
        while True:
            n = next(gen)
            if n is not None:
                yield n
            else:
                break
        yield gen.send([', '.join(str(i) for i in self.objects)])
        while True:
            n = next(gen)
            if n is not None:
                yield n
            else:
                break
        yield gen.send(global_vars(self.vars.values(), 1, len(self.objects)))
        while True:
            n = next(gen)
            if n is not None:
                yield n
            else:
                break
        yield gen.send(SolvingStep(i).write(self.rules) for i in self.steps)
        yield from gen

    def __repr__(self):
        return f'watched: {self.watched}\nvars: {self.vars}\nrules:\n' + '\n'.join(
            repr(i) for i in self.rules)

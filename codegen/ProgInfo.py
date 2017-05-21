################################################################################
# This file is part of ATOM
# Copyright (c) 2017 by Yifei Zheng
# Unauthorized copy, distribution and modification of this file is prohibited.
#
# This file contains structures to store data from source code, and useful
# processing routines for code generation.
################################################################################
import sys, os.path
from . import templating

sys.path.insert(0, os.path.abspath('..'))
from parser import expr
from collections import OrderedDict as odict
from itertools import chain, islice
from . import resolver


def union_all(iterable):
    a = set()
    for i in iterable:
        a.update(i)
    return a


def removeDup(seq):
    return list(odict.fromkeys(seq).keys())


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


class FreeVar:
    def __init__(self, name, type, initializer=None):
        self.name = name
        self.type = type
        self.value = initializer


class Op:
    def __init__(self, name, argc=1, writer=None):
        self.name = name
        self.argc = argc
        self.writer = writer

    def write(self, subwriter, args):
        if self.writer:
            return self.writer(subwriter, args)
        else:
            return '{}({})'.format(self.name, ', '.join(subwriter(arg) for arg in args))

    def __repr__(self):
        return 'Op:' + self.name


def diff_writer(args, mixin):
    args = list(listVars(args[0]))
    lambda_args = ['double ' + i for i in args]
    calling_args = ['get_at_time(history, time_).get(comb_.get(0)).' + i for i in args]
    calling_args_prev = ['get_at_time(history, time_ - step).get(comb_.get(0)).' + i for i in args]
    return f'[]({", ".join(lambda_args)}){{ return {mixin(args[0])}; }}({", ".join(calling_args)}) - []({", ".join(lambda_args)}){{return {mixin(args[0])}; }}({", ".join(calling_args_prev)})'


OP_TO_INST = {
    '+': Op('math::op::add', 2),
    '-': Op('math::op::sub', 2),
    '*': Op('math::op::mul', 2),
    '/': Op('math::op::div', 2),
    'pow': Op('std::pow', 2),
    'sin': Op('std::sin', 1),

    'sqrt': Op('math::sqrt', 1),
    'diff': Op('diff', 1, diff_writer),
}


class SolvingStep:
    def __init__(self, content):
        self.content = content
        if isinstance(self.content[0], list):
            self.type = 0
        elif isinstance(self.content[0], int):
            self.type = 1
        else:
            self.type = 2

    def write(self, space, pack):
        def cat(t):
            return f'{t.name}_{t.order}'

        def prev(t):
            return t + '_prev'

        if self.type == 0:
            return f'math::solver::solve_algebraic_sys({pack[self.content[0]]})\n'
        elif self.type == 1:
            if self.content[1].order == 0:
                name = self.content[1].name
                binding = f'double& {name} = history.back().second->get(comb_.get(0)).{name};\n'
                binding += f'double {prev(name)} = {name};\n'
            else:
                name = cat(self.content[1])
                binding = f'double {name};\n'
            seed = name
            return binding + f'{name} = math::solver::solve_algebraic_single([&](double {name}){{\n' + pack[self.content[0]].write() + f'}}, {seed});\n'
        else:
            if self.content[1].order == 0:
                name = self.content[1].name
                binding = f'double& {name} = history.back().second->get(comb_.get(0)).{name};\n'
                binding += f'double {prev(name)} = {name};\n'
            else:
                binding = f'double {cat(self.content[0])};\n'
            if self.content[1].order > self.content[0].order:
                return binding + f'{name} = ({cat(self.content[0])} - {cat(self.content[0])}.prev()) / step;\n'
            else:
                return binding + f'{name} += {cat(self.content[0])} * step;\n'

    def __str__(self):
        if self.type == 0:
            return f'ALG_M {self.content}'
        elif self.type == 1:
            return f'ALG_S {self.content[0]} -> {self.content[1]}'
        else:
            return f'DIFF_S {self.content[0]} -> {self.content[1]}'

    @property
    def update(self):
        if self.type == 0:
            return self.content[1]
        else:
            return [self.content[1]]


def processSteps(rules, watches, steps):
    all_vars = union_all(RuleResolvingStruct(0, rule).diffs for rule in rules)
    updated_VAR = set()  # this ignores the order
    requires_further_work = set()
    for i in steps:
        for var in i.update:
            if var.order == 0:
                updated_VAR.add(var.name)
            else:
                requires_further_work.add(var.name)
    requires_further_work.intersection_update(watches)
    # build index for the variables that requires further work
    NVar = resolver.cNVar
    for var in requires_further_work:
        forms = [i.order for i in all_vars if i.name == var]
        forms.remove(0)
        steps.append(SolvingStep((NVar(var, min(forms)), NVar(var, 0))))


class Rule:
    """Simple structure that stores a rule"""
    ITER = 1
    VAR_FINDER = None

    def __init__(self, content):
        self.content = expr.stringToExpr(content[0]), expr.stringToExpr(content[1])

    @staticmethod
    def writeExpr(expression: tuple):
        if isinstance(expression, str):
            var = expression
            num = 0
            if '$' in var:
                name, var = var[1:].split('.', 1)
                try:
                    num = int(name)
                    Rule.ITER = max(Rule.ITER, num)
                except ValueError:
                    pass
            category = Rule.VAR_FINDER(var)
            if category == 0:  # global, direct output
                return var
            elif category == 1:  # watched, fetch and output
                return f'last_data_.get(comb_.get({num})).' + var
            else:  # tmp, direct output
                return var
        elif isinstance(expression, int):
            return str(expression)
        else:
            return OP_TO_INST[expression[0]].write(Rule.writeExpr, expression[1])

    def write(self):
        return 'return math::op::sub(' + self.writeExpr(self.content[0]) + ',\n' + self.writeExpr(
            self.content[1]) + ');\n'

    def __repr__(self):
        return f'Rule {self.content[0]} = {self.content[1]}'


class RulePack:
    """
    This is the class representing packed rules (RuleResolvingStruct) on
    highest level. That is, the pack is resolved further to actual steps,
    e.g. to solve as ODE or algebraic system, etc.
    """

    def __init__(self, content, solveFor):
        self.pack = content
        self.all_forms_dict = {}
        for rule in self.pack:
            for var in rule.diffs:
                self.all_forms_dict.setdefault(var.name, []).append(var.order)
        self.solveFor = solveFor
        self.steps = None

    def resolve(self, known):
        known = set(known).difference(i.name for i in self.solveFor)
        for i in self.pack:
            i.minus(known)

        self.steps = resolver.resolve(self.pack, self.all_forms_dict)

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
    """Structure that holds necessary information to resolve rules to steps."""

    def __init__(self, idx, rule):
        self.idx = idx
        self.rule = rule
        self.vars = set(chain(listVars(rule.content[0]), listVars(rule.content[1])))
        self.diffs = set(chain(findDerivatives(rule.content[0]), findDerivatives(rule.content[1])))

    def minus(self, set):
        self.vars = {i for i in self.vars if i not in set}
        self.diffs = {i for i in self.diffs if i.name not in set}

    def __repr__(self):
        return repr(self.rule)


class Space:
    def __init__(self):
        self.watched = odict()
        self.objects = odict()
        # FIXME: static initialization fiasco, shall be ordered instead
        self.globals = {"t": FreeVar('t', 'double', 1), "step": FreeVar('step', 'double', 1e-3)}
        self.vars = odict()
        self.funcs = {}  # reserved
        self.rules = []
        self.steps = None

    # FIXME: issue a warning instead of silent overwriting
    def addWatch(self, name, type='double'):
        self.watched[name] = FreeVar(name, type)

    def addObj(self, name, init=[]):
        self.objects[name] = FreeVar(name, 'Object', init)

    def declTempVar(self, name, type):
        self.vars[name] = type

    def addRule(self, content):
        self.rules.append(Rule(content))

    def addCondRule(self, *args):
        self.rules.append(CondRule(args))

    def find_var(self, varname):
        """
        The return code of this function shall be consistent with those used
        in Rule.writeExpr.
        """
        if varname in self.globals:
            return 0
        elif varname in self.watched:
            return 1
        elif varname in self.vars:
            return 0

    def process(self):
        print("Processing...")
        known = list(chain(self.watched, self.globals))
        rrs = [RuleResolvingStruct(idx, i) for idx, i in enumerate(self.rules)]

        def find_candidate(known, all_vars):
            for i in known:
                for j in all_vars:
                    if j.name == i:
                        yield j

        def rotate(known, update):
            for i in update:
                try:
                    known.remove(i.name)
                    known.append(i.name)
                except ValueError:
                    pass

        # must loop by reference
        idx = 0
        while idx < len(rrs):
            Idx = idx
            eqn_count = 0
            needMore = True
            while needMore:
                Idx += 1
                eqn_count += 1
                # validate selection here
                all_vars = union_all(i.diffs for i in rrs[idx:Idx])
                update = {var for var in all_vars if var.name not in known}
                needMore = len(update) > eqn_count
            update.update(islice(find_candidate(known, all_vars), eqn_count - len(update)))
            rrs[idx:Idx] = [RulePack(rrs[idx:Idx], update)]
            rrs[idx].resolve(known)
            rotate(known, update)
            idx = Idx

        self.steps = removeDup(step for pack in rrs for step in pack.steps)
        self.steps = [SolvingStep(i) for i in self.steps]
        processSteps(self.rules, self.watched, self.steps)

    def write(self):
        # FIXME: This section should be generated
        def global_vars(var_vals, iterlen, object_len):
            for i in var_vals:
                yield f'{i.type} {i.name} = {i.value};'
            yield f'combinations<{iterlen}> comb_(0, {object_len});'

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
        yield gen.send(self.objects)
        while True:
            n = next(gen)
            if n is not None:
                yield n
            else:
                break
        yield gen.send(f'obj.{var.name}' for var in self.watched.values())
        while True:
            n = next(gen)
            if n is not None:
                yield n
            else:
                break
        yield gen.send(', '.join(str(i) for i in obj.value) for obj in self.objects.values())
        while True:
            n = next(gen)
            if n is not None:
                yield n
            else:
                break
        yield gen.send(global_vars(self.globals.values(), 1, len(self.objects)))
        while True:
            n = next(gen)
            if n is not None:
                yield n
            else:
                break
        yield gen.send([f'comb_.get(0) + 1 != {len(self.objects)}'])
        while True:
            n = next(gen)
            if n is not None:
                yield n
            else:
                break
        print('\n'.join(str(i) for i in self.steps))
        yield gen.send(i.write(self, self.rules) for i in self.steps)
        yield from gen

    def __repr__(self):
        return f'watched: {self.watched}\nvars: {self.globals}\nrules:\n' + '\n'.join(
            repr(i) for i in self.rules)

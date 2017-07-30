################################################################################
# This file is part of ATOM
# Copyright (c) 2017 by Yifei Zheng
# Unauthorized copy, distribution and modification of this file is prohibited.
#
# This file contains structures to store data from source code, and useful
# processing routines for code generation.
#
# WARNING ## DATA CORRUPTION MAY OCCUR ## CAUTION
# DO NOT RUN COMPILER IN PARALLEL IN CURRENT RELEASE
# THE FILE CONTAINS BAD USAGE PATTERN OF GLOBAL VARIABLE USED FOR TESTING.
################################################################################
import sys, os.path
from contextlib import contextmanager, ExitStack
from . import resolver
from . import manglers
from . import templating

sys.path.insert(0, os.path.abspath('..'))
from parser import expr
from collections import OrderedDict as odict
from itertools import chain, islice


def removeDup(seq):
    return list(odict.fromkeys(seq).keys())


class Expr:
    VAR_WRITER = {}

    def __init__(self, expr):
        if isinstance(expr, tuple):
            self.content = (expr[0], [Expr(i) for i in expr[1]])
        else:
            self.content = expr

    def listVars(self):
        if isinstance(self.content, tuple):
            for i in self.content[1]:
                yield from i.listVars()
        elif isinstance(self.content, str):
            yield self.content

    def findDerivatives(self, DIFF=0):
        if isinstance(self.content, tuple):
            if self.content[0] == 'diff':
                DIFF += 1
            for i in self.content[1]:
                yield from i.findDerivatives(DIFF)
            if self.content[0] == 'diff':
                DIFF -= 1
        elif isinstance(self.content, str):
            yield resolver.cNVar(self.content, DIFF)

    @staticmethod
    def proper_write_var(var):
        override = Expr.VAR_WRITER.get(var)
        if override:
            return override
        if isinstance(var, resolver.cNVar):
            if var.order == 0:
                return Expr.proper_write_var(var.name)
            else:
                return Expr.proper_write_var(manglers.cNVar(var))
        try:
            return manglers.mem_last(var)
        except ValueError:
            return var

    @staticmethod
    def lambda_arg(var):
        if isinstance(var, resolver.cNVar):
            if var.order == 0:
                return Expr.lambda_arg(var.name)
            else:
                return Expr.lambda_arg(manglers.cNVar(var))
        try:
            return manglers.strip_mem(var)
        except ValueError:
            return var

    def write(self):
        if isinstance(self.content, str):
            return Expr.proper_write_var(self.content)
        elif isinstance(self.content, int):
            return str(self.content)
        else:
            return OP_TO_INST[self.content[0]].write(self.content[1])

    def __repr__(self):
        return repr(self.content)


class Space:
    def __init__(self):
        self.watched = odict()
        self.objects = odict()
        # FIXME: static initialization fiasco, shall be ordered instead
        self.globals = {
            "t": Variable('t', 'double', Variable.CTX_GLOBAL, 1),
            "step": Variable('step', 'double', Variable.CTX_GLOBAL, 1e-3)
        }
        self.vars = odict()
        self.funcs = {}  # reserved
        self.rules = []
        self.steps = None

    # FIXME: issue a warning instead of silent overwriting
    def addWatch(self, name, type='double'):
        self.watched[manglers.generic_mem(name)] = Variable(name, type, Variable.CTX_MEM)

    def addObj(self, name, init):
        self.objects[name] = Variable(name, 'Object', Variable.CTX_GLOBAL, init)

    def declTempVar(self, name, type):
        self.vars[name] = type

    def addRule(self, content):
        self.rules.append(Rule(content))

    def addCondRule(self, *args):
        self.rules.append(CondRule(args))

    def process(self):
        print("Processing...")
        known = list(chain(self.watched, self.globals))
        rrs = [RuleResolvingStruct(idx, i) for idx, i in enumerate(self.rules)]

        def find_candidate(known, all_vars):
            for i in known:
                for j in all_vars:
                    if manglers.var_match(i, j.name):
                        yield j

        def rotate(known, update):
            for i in update:
                generic_name = manglers.generic_mem(manglers.strip_mem(i.name))
                try:
                    known.remove(generic_name)
                    known.append(generic_name)
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
                all_vars = {var for i in rrs[idx:Idx] for var in i.diffs}
                update = {var for var in all_vars if
                          not any(manglers.var_match(kvar, var.name) for kvar in known)}
                needMore = len(update) > eqn_count
            update.update(islice(find_candidate(known, all_vars), eqn_count - len(update)))
            rrs[idx:Idx] = [RulePack(rrs[idx:Idx], update)]
            rrs[idx].resolve(known)
            rotate(known, update)
            idx = Idx

        self.steps = removeDup(step for pack in rrs for step in pack.steps)
        self.steps = [SolvingStep(i) for i in self.steps]

        # finalize the steps, ensure update are propagated to watched values.
        updated_variables = {var: idx for idx, step in enumerate(self.steps) for var in
                             step.updates}
        update_not_propagated = set()
        for NVar in updated_variables:
            var, form = NVar.name, NVar.order
            generic_name = manglers.generic_mem(manglers.strip_mem(var))
            if form != 0 and generic_name in self.watched:
                update_not_propagated.add(NVar)
        # propagate the update to its watched base
        for NVar in update_not_propagated:
            idx = updated_variables[NVar]
            base_step = self.steps[idx]
            self.steps[idx] = SolvingStep((base_step, NVar.name))

    def write(self):
        # FIXME: This section should be generated
        def global_vars(var_vals, iterlen, object_len):
            for i in var_vals:
                yield f'{i.type} {i.name} = {i.value};'
            yield f'combinations<{iterlen}> comb_(0, {object_len});'

        print('Generating code...')
        gen = templating.template('codegen/template')

        def consume(gen, send):
            while True:
                n = next(gen)
                if n is not None:
                    yield n
                else:
                    yield gen.send(send)
                    break

        yield from consume(gen, (f'{var.type} {var.name};' for var in self.watched.values()))
        yield from consume(gen, self.objects)
        yield from consume(gen, (f'obj.{var.name}' for var in self.watched.values()))
        yield from consume(gen,
                           (', '.join(str(i) for i in obj.value) for obj in self.objects.values()))
        yield from consume(gen, global_vars(self.globals.values(), 1, len(self.objects)))
        yield from consume(gen, (i.write(self, self.rules) for i in self.steps))
        yield from gen

    def __repr__(self):
        return f'watched: {self.watched}\nvars: {self.globals}\nrules:\n' + '\n'.join(
            repr(i) for i in self.rules)


class Variable:
    CTX_GLOBAL = 0
    CTX_MEM = 1
    CTX_TMP = 2

    def __init__(self, name, type, ctx, initializer=None):
        self.name = name
        self.type = type
        self.value = initializer


# WE LEAVE A GLOBAL VARIABLE HERE WHICH IS BAD
# TODO: ONCE TESTED, ENCAPSULATE IT IN A CLASS
STEP_BYPRODUCT = {}


class Op:
    def __init__(self, name, argc=1, writer=None):
        self.name = name
        self.argc = argc
        self.writer = writer

    def write(self, args):
        if self.writer:
            return self.writer(args)
        else:
            return '{}({})'.format(self.name, ', '.join(Expr.write(arg) for arg in args))

    def __repr__(self):
        return 'Op:' + self.name


@contextmanager
def write_var_as(on, replace):
    old = Expr.VAR_WRITER.get(on)
    Expr.VAR_WRITER[on] = replace
    yield
    if old:
        Expr.VAR_WRITER[on] = old
    else:
        del Expr.VAR_WRITER[on]


def lambdify(expr, vars):
    """
    Convert an expression into lambda. Caller shall ensure all arguments of the lambda shall be
    compatible with target naming restrictions. All arguments are written through proper_write_var.
    """
    lambda_args = ', '.join('double ' + Expr.proper_write_var(i) for i in vars)
    return f'[]({lambda_args}){{ return {Expr.write(expr)}; }}'


DIFF_CHAIN = set()


@contextmanager
def register_diff_chain_wrt(var):
    global DIFF_CHAIN
    DIFF_CHAIN.add(var)
    yield
    DIFF_CHAIN.remove(var)


def diff_writer(args):
    if len(args) == 1:
        expr = args[0]
        wrt = 't'
    else:
        expr, wrt = args
    if wrt != 't':
        raise NotImplementedError()
    diffs = set(expr.findDerivatives())
    chain = set()
    for var in DIFF_CHAIN:
        test = resolver.cNVar(var.name, var.order - 1)
        if test in diffs:
            chain.add((var, test))
    if chain:
        flambda = lambdify(expr, {i[0].name for i in chain})
        if len(chain) > 1:  # FIXME: Multi-dimensional chain rule broken
            raise NotImplementedError()
        vdiff, vnow = chain.pop()
        return f'math::calculus::fdiff({flambda}, {Expr.proper_write_var(vnow)}) * {Expr.proper_write_var(vdiff)}'
    else:
        fnow = Expr.write(expr)
        with ExitStack() as stack:
            # FIXME: This is still broken if expression involves variable and its derivative
            vars = set(expr.listVars())
            for i in vars:
                stack.enter_context(write_var_as(i, Expr.lambda_arg(i)))
            flambda = lambdify(expr, vars)
        fvars = [Expr.proper_write_var(i) for i in vars]
        # FIXME: round away from zero only, should be configurable
        args = ', '.join(f'take_step({i}, {i})' for i in fvars)
        return f'({flambda}, {args} - {fnow}) / step'


OP_TO_INST = {
    '+': Op('math::op::add', 2),
    '-': Op('math::op::sub', 2),
    '*': Op('math::op::mul', 2),
    '/': Op('math::op::div', 2),
    'pow': Op('std::pow', 2),
    'sin': Op('std::sin', 1),
    'cos': Op('std::cos', 1),
    'tan': Op('std::tan', 1),
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
        STEP_START = ''
        STEP_END = ''
        if self.type == 0:
            return STEP_START + f'math::solver::algebraic_sys({pack[self.content[0]]});\n' + STEP_END
        elif self.type == 1:
            rule_id, solveFor = self.content
            name = solveFor.name
            with write_var_as(name, manglers.fetch_mem(name)):
                if solveFor.order == 0:  # an ultimate request
                    # TODO: This part is broken
                    bind = manglers.mem_cur(name)
                    seed = manglers.mem_last(name)
                    return STEP_START + f'{bind} = math::solver::algebraic_single([&](double {name}){{\n' + \
                           pack[rule_id].write() + f'}}, {seed});\n' + STEP_END
                else:  # request from diff
                    # chain rule must be considered if the unknown is a derivative.
                    # See make_cs_diff_writer for implementation
                    seed = 0
                    with write_var_as(solveFor, Expr.lambda_arg(solveFor)), register_diff_chain_wrt(solveFor):
                        return f'[&](double t, double {Expr.proper_write_var(name)}){{' \
                               + f'return math::solver::algebraic_single([&](double {Expr.proper_write_var(solveFor)}){{\n' \
                               + pack[rule_id].write() + f'}}, {seed}); }}'
        else:
            base, var = self.content
            return STEP_START + f'{manglers.mem_cur(var)} = math::solver::differential(\n' \
                   + f'{base.write(space, pack)}, time_, {manglers.mem_last(var)}, step).first;\n' + STEP_END

    def __str__(self):
        if self.type == 0:
            return f'ALG_M {self.content}'
        elif self.type == 1:
            return f'ALG_S {self.content[0]} -> {self.content[1]}'
        else:
            return f'DIFF_S ({self.content[0]}) -> {self.content[1]}'

    @property
    def updates(self):
        if self.type == 0:
            return self.content[1]
        else:
            return [self.content[1]]


class Rule:
    """Simple structure that stores a rule"""

    def __init__(self, content):
        self.lhs, self.rhs = Expr(expr.stringToExpr(content[0])), Expr(expr.stringToExpr(content[1]))

    def write(self):
        return 'return math::op::sub(' + self.lhs.write() + ',\n' + self.rhs.write() + ');\n'

    def __repr__(self):
        return f'Rule {self.lhs} = {self.rhs}'


class RulePack:
    """
    This is the class representing packed rules (RuleResolvingStruct) on
    highest level. That is, the pack is resolved further to actual steps,
    e.g. to solve as ODE or algebraic system, etc.
    """

    def __init__(self, content, solveFor):
        self.pack = content
        self.solveFor = solveFor
        self.steps = None

    def resolve(self, known):
        to_remove = {var for var in known if not any(manglers.var_match(svar.name, var) for svar in self.solveFor)}
        for i in self.pack:
            # exclude known variables
            i.diffs = {var for var in i.diffs if not any(manglers.var_match(kvar, var.name) for kvar in to_remove)}

        self.steps = resolver.resolve(self.pack, self.solveFor)

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
                yield from Expr.write(expr.stringToExpr(lhs))
                yield from Expr.write(expr.stringToExpr(rhs))

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
        self.diffs = set(chain(rule.lhs.findDerivatives(), rule.rhs.findDerivatives()))

    def __repr__(self):
        return repr(self.rule)

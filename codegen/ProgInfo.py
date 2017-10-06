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
from collections import OrderedDict as odict
from . import warning
from . import templating
from . import resolver
from .support import cSrcLocation as src_tracker, cTypes, cVariable as Var
from itertools import chain
from contextlib import contextmanager
import sys, os
sys.path.insert(0, os.path.abspath('..'))
from parser import expr


class Flag:
    def __init__(self, val):
        self.flag = val

    def __bool__(self):
        return self.flag


class CondRuleType:
    # This has the same interface as RuleType, should have an abstract base
    def __init__(self, src, dom, *condrulepair):
        self.src = src
        self.dom = dom
        self.crpack = condrulepair
        self.diffs = {rule[0].findDerivatives() for _, rule in condrulepair}
        self.diffs.update(rule[1].findDerivatives() for _, rule in condrulepair)

    def list_vars(self):
        return {i.name for i in self.diffs}


class ObjType:
    def __init__(self, src, vals):
        self.src = src
        self.vals = vals


class DeclType:
    def __init__(self, src, types):
        self.src = src
        self.types = types


class Op:
    def __init__(self, name, argc=1, writer=None):
        self.name = name
        self.argc = argc
        self.writer = writer

    def __repr__(self):
        return 'Op:' + self.name


def diff_writer(args, default_writer: 'StepWriter'):
    expr, *wrt = args
    if wrt:
        raise NotImplementedError
    all_vars = set(resolver.cNVar(i.name, i.order[0]+1) for i in expr.findDerivatives())
    solving = {var for var in default_writer.state[-1] if var in all_vars}
    if not solving:
        varlist = list(expr.listVars())
        with default_writer.VarWriter.tmp_override({i:default_writer.write_arg(i) for i in varlist}):
            return 'math::calculus::fdiff(' \
               + default_writer.write_lambda("&", varlist, "return " + default_writer.write_expr(expr) + ";") \
               + ", step, " + ", ".join(default_writer.write_getter(i) for i in varlist) + ')'
    else:
        # When using chain-rule we have to adjust writing scheme for base
        # variables as well
        var = solving.pop()
        if var.order[0] > 1:
            var_base = resolver.cNVar(var.name, var.order[0] - 1)
        else:
            var_base = var.name
        original_write = default_writer.VarWriter.write_value(var_base)
        with default_writer.VarWriter.tmp_override({var_base: default_writer.write_arg(var_base)}):
            text = f'math::calculus::fdiff({default_writer.write_lambda("&", [var_base], "return " + default_writer.write_expr(expr) + ";")}, {original_write})'
            return text + f' * {default_writer.VarWriter.write(var)}'

OP = {
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


class Expr:
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

    def __repr__(self):
        return repr(self.content)


class StepWriter:
    def __init__(self, space: 'Space'):
        self.space = space
        self.state = []
        self.VarWriter = StepWriter.VarWriter(self.space.locate_var)
        self.noBind = Flag(False)

    @staticmethod
    @contextmanager
    def tmp_flag(flag, a):
        old = flag.flag
        flag.flag = a
        yield
        flag.flag = old

    def write(self, step):
        use, update = step
        if isinstance(use, int):
            if self.noBind:
                bind, end = '', ''
            else:
                bind = f'{self.VarWriter.write(update)} = '
                end = ';'
            if update.order[0] == 0:
                update = update.name
            with self.VarWriter.tmp_override({update: self.write_arg(update)}):
                return bind + self.write_alg(
                    self.write_lambda('&',
                                  [update],
                                  f'return {self.write_rule(self.space.rules[use])};'),
                                  self.VarWriter.write_value(update)) + end
        elif isinstance(use, tuple):
            if self.noBind:
                bind, end = '', ''
            else:
                bind = f'{self.VarWriter.write(update)} = '
                end = ';'
            update_intermediate = use[1]
            self.state.append([update_intermediate])
            with self.tmp_flag(self.noBind, True), self.tmp_flag(self.VarWriter.ForceValue, True),\
                    self.VarWriter.tmp_override({update_intermediate: self.write_arg(update_intermediate)}):
                return bind + self.write_diff(
                    self.write_lambda('&', ['t', update_intermediate], f'return {self.write(use)};'),
                    't', self.VarWriter.write_value(update),
                    'step') + '.first' + end
        else:
            bind = ''
            end = '' if self.noBind else ';'
            return bind + self.write_alg_sys(', '.join(self.VarWriter.write(i) for i in update),
                                             ', '.join(self.space.rules[i] for i in use),
                                             ) + end

    @staticmethod
    def write_alg(rule, seed):
        return f'math::solver::algebraic({rule}, {seed})'

    @staticmethod
    def write_alg_sys(vars, rules):
        return f'math::solver::algebraic_sys({{{vars}}}, {rules})'

    @staticmethod
    def write_diff(gen, wrt, seed, step):
        return f'math::solver::differential({gen}, {wrt}, {seed}, {step})'

    def write_rule(self, rule):
        return f'math::op::sub({self.write_expr(rule.lhs)}, {self.write_expr(rule.rhs)})'

    def write_expr(self, expr):
        if isinstance(expr, Expr):
            expr = expr.content
        if isinstance(expr, tuple):
            op = OP[expr[0]]
            if op.writer:
                return op.writer(expr[1], self)
            else:
                return f'{op.name}({", ".join(self.write_expr(i) for i in expr[1])})'
        elif isinstance(expr, str) or isinstance(expr, resolver.cNVar):
            return self.VarWriter.write(expr)
        else:
            return str(expr)

    class VarWriter:
        def __init__(self, parent):
            # controls writing of unrecorded variable
            # i.e. the variables computed on demand
            self.look_up = parent
            self.ForceValue = Flag(False)
            self.override = {} # primarily used for lambda

        @contextmanager
        def tmp_override(self, iter):
            old = self.override.copy()
            self.override.update(iter)
            yield
            self.override = old

        def write(self, var):
            if var in self.override:
                return self.override[var]
            elif isinstance(var, resolver.cNVar):
                if sum(var.order) != 0:
                    return f'{self.write(var.name)}_{var.order[0]}'
                else:
                    return self.write(var.name)
            else:
                lookup_result = self.look_up(var)
                if isinstance(lookup_result, tuple):
                    return self.write_watch_new(lookup_result[0], lookup_result[1])
                else:
                    return var

        def write_value(self, var):
            if isinstance(var, resolver.cNVar):
                if sum(var.order) != 0:
                    return 0  # TODO: Check for step by-product of differential solver
                else:
                    return self.write_value(var.name)
            else:
                lookup_result = self.look_up(var)
                if isinstance(lookup_result, tuple):
                    return self.write_watch_last(lookup_result[0], lookup_result[1])
                else:
                    return var

        @staticmethod
        def write_watch_new(num, var):
            return f'srd_.get({num}).{var}'

        @staticmethod
        def write_watch_last(num, var):
            return f'last_data_.get({num}).{var}'

    def write_lambda(self, capture, args, body):
        return f'[{capture}]({", ".join(self._write_arg(i) for i in args)}){{{body}}}'

    def _write_arg(self, var):
        if isinstance(var, resolver.cNVar):
            if sum(var.order) != 0:
                # FIXME: Duplicate code
                lookup_result = self.space.locate_var(var.name)
                assert isinstance(lookup_result, tuple) # must be a watch to be differentiated
                return f'{lookup_result[2]} {var.name.split(".", 1)[1]}_{var.order[0]}'
            else:
                var = var.name

        lookup_result = self.space.locate_var(var)
        if isinstance(lookup_result, tuple):
            return f'{lookup_result[2]} {lookup_result[1]}'
        else:
            return f'{lookup_result.type.base} {var}'

    def write_arg(self, var):
        return self._write_arg(var).split(' ', 1)[1]

    def write_getter(self, var):
        """write a functor that can get all history values of watch of a specific object"""
        lookup_result = self.space.locate_var(var)
        assert isinstance(lookup_result, tuple)
        offset = f'offsetof(object_, {lookup_result[1]})'
        return f'getter_<{lookup_result[0]}, {offset}, {lookup_result[2]}>(history)';

    def __iter__(self):
        for step in self.space.steps:
            yield self.write(step)


class Space:

    def __init__(self):
        self.objs = odict()
        self.watches = odict()
        self.globals = {'t': Var(cTypes(cTypes.BaseEnum.DOUBLE, True, 0), 1),
                        'step': Var(cTypes(cTypes.BaseEnum.DOUBLE, False, 0), 0.001)}
        self.tmps = {}
        self.rules = []
        self.loopctl = set()
        self.steps = None

    def addRule(self, rule, dom=None, src=None):
        self.rules.append(
            resolver.cRule(src if src else src_tracker(0, 0), dom, rule[0], rule[1]))

    def addObj(self, obj, vals, src=None):
        if obj in self.objs:
            if not warning.warn_ask(f'Object named {obj} exists, continue?'):
                return
        self.objs[obj] = ObjType(src if src else src_tracker(0, 0), vals)

    def addTmp(self, var, types, src=None):
        self.tmps[var] = DeclType(src, types)

    def addLoopctl(self, var, src):
        pass

    def addWatch(self, var, types, src=None):
        if var in self.watches:
            if not warning.warn_ask(f'Watch {var} exists, continue?'):
                return
        self.watches[var] = cTypes(types, False, 0)

    def processRules(self):

        #TODO: Check consistency of boundary values first

        def is_known(NVar):
            return True # FIXME

        def need_update(NVar):
            v = self.locate_var(NVar.name)
            return isinstance(v, tuple) # FIXME

        def list_der(x):
            for i in Expr(expr.stringToExpr(x)).findDerivatives():
                i.can_start = is_known(i)
                i.need_update = need_update(i)
                yield i

        self.steps = resolver.resolve(self.rules, list_der)

        # propagate updates here
        updated = {}
        for i, (_, var) in enumerate(self.steps):
            if var.order[0] < updated.get(var.name, (None, [256]))[1][0]: # FIXME: Magic value, shall be inf
                updated[var.name] = i, var.order
        for var, (i, order) in updated.items():
            if sum(order) != 0:
                self.steps[i] = self.steps[i], resolver.cNVar(var, 0)

    def write(self):
        gen = templating.template('codegen/template')
        consume = templating.consume
        yield from consume(gen, (f'{type} {name};' for name, type in self.watches.items()))
        yield from consume(gen, self.objs)
        yield from consume(gen, (f'obj.{var}' for var in self.watches))
        yield from consume(gen,
                           (', '.join(str(i) for i in obj.vals) for obj in self.objs.values()))
        yield from consume(gen, (f'{tv.type} {name} = {tv.val};' for name, tv in self.globals.items()))
        yield from consume(gen, [f'combinations<1> comb_(0, {len(self.objs)});'])  # FIXME: insert true value when tested
        writer = StepWriter(self)
        yield from consume(gen, writer)
        yield from gen

    def locate_var(self, var):
        if var.startswith('$'):
            obj, watch = var[1:].split('.', 1)
            base_watch = self.watches[watch]
            return int(obj) if obj.isdigit() else obj, watch, base_watch
        elif var in self.tmps:
            return self.tmps[var]
        elif var in self.globals:
            return self.globals[var]
        else:
            return self.objs[var]

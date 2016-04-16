"""Generic Math Processing
"""
from AhoCorasick import ACProcessor
from error import UnspecifiedVariable, MathError
import math
from itertools import product as cartesian
from collections import OrderedDict as odict
from string import Template
from operator import add, sub, mul, truediv
from copy import deepcopy as copy


class operator:
    """Base class for all kinds of operators.
    """
    __slots__ = ('proc', 'priority', 'alt')

    def __init__(self, proc, priority, equivalent={}):
        self.proc = proc
        self.priority = priority
        self.alt = equivalent

    def __call__(self, *args):
        return self.proc(*args)

OP_PLUS = operator(add, 1, {lambda expr:
                            plexpr(arg1=expr.arg2, arg2=expr.arg1)})
OP_MINUS = operator(sub, 1)
OP_MUL = operator(mul, 2, {lambda expr:
                           plexpr('*', arg1=expr.arg2, arg2=expr.arg1)})
OP_SPACE_MUL = operator(mul, 2)
OP_DIV = operator(truediv, 2)
OP_POW = operator(pow, 3)


class RewritingRule:

    def __init__(self, op, cond, reduceto):
        self.cond = cond
        self.reduceto = reduceto

    def verify(self, expr):
        try:
            return self.cond(expr)
        except:
            return False

    def rewrite(self, expr):
        return self.reduceto(expr)

# ----------------------------------------------------------------------------
# DEFINE REWRITING RULES HERE
# The following lines use RR as abbreviation to rewriting rule.
RR_PLUS_0 = RewritingRule('+', lambda expr: 0 in (expr[1], expr[2]),
                          lambda expr:
                          expr[1] if expr[1] else expr[2])
RR_MUL_0 = RewritingRule('*', lambda expr: 0 in (expr[1], expr[2]),
                         lambda expr: 0)
RR_MUL_1 = RewritingRule('*', lambda expr: 1 in (expr[1], expr[2]),
                         lambda expr:
                         expr[2] if expr[1] is 1 else expr[1])
RR_MUL_TOPOW = RewritingRule('*', lambda expr: expr[1] is expr[2],
                             lambda expr:
                             plexpr('^', expr[1], 2))
RR_MUL_POW = RewritingRule('*', lambda expr: expr[1] is expr[2][1],
                           lambda expr:
                           plexpr('^', expr[1], expr[2][2] + 1))
RR_DIV_MUL = None
RR_DICT = {'+': {RR_PLUS_0},
           '*': {RR_MUL_0, RR_MUL_1, RR_MUL_TOPOW},
           '/': {}}
# ----------------------------------------------------------------------------

class plexpr:  # polish notation

    output = Template('${op}(${arg1}, ${arg2})')
    opfuncmap = {'+': add, '-': sub, '*': mul, '/': truediv, '^': pow,
                 'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
                 'sinh': math.sinh, 'cosh': math.cosh, 'tanh': math.tanh,
                 'ln': math.log, 'sqrt': math.sqrt}

    def __init__(self, operator='+', arg1=0, arg2=0):

#        assert operator in plexpr.opfuncmap
        self.stroperator = operator
        self.priority = strexpr.priority.get(operator, 4)
        self.arg1 = arg1
        self.arg2 = arg2
#         self.maxlevel = 1
#         self.cursor = [2]  # cursor are initialized to point to arg2
        self.level = 1

    def get(self, cursor):
        result = self
        for i in cursor:
            result = result[i]
        return result

    def set(self, cursor, value):
        result = self
        if cursor:
            for i in cursor[:-1]:
                result = result[i]
            result[cursor[-1]] = value

    def isEnd(self):
        # recursive code need optimization
        result = self
        for i in range(self.level):
            result = result[2]
        if type(result) is plexpr:
            i = 2
            while result is not None:
                if type(result[i]) in {int, float, str}:
                    return True
                result = result[i]
            return False
        return True

    # the follwing four are shortcuts

    @property
    def operate(self):
        return plexpr.opfuncmap[self.stroperator]

    @property
    def req(self):
        result = self
        for _ in range(self.level):
            result = result[2]
        return result

    @req.setter
    def req(self, value):
        result = self
        for _ in range(self.level - 1):
            result = result[2]
        result[2] = value

    @property
    def reqexpr(self):
        result = self
        for _ in range(self.level - 1):
            result = result[2]
        return result

    @reqexpr.setter
    def reqexpr(self, value):
        if self.level > 1:  # equivalent to >=2
            result = self
            for _ in range(self.level - 2):
                result = result[2]
            result[2] = value
        elif self.level:
            self[0] = value[0]
            self[1] = value[1]
            self[2] = value[2]

    @property
    def reqexprop(self):  # return the immediate opearator of the req
        return self.reqexpr.stroperator

    def append(self, other, refop=None, bovrd=False, reflevel=0, bsovrd=False):
#         print(self, other, self.level, reflevel, sep='|')
        if other in strexpr.operators:  # token detected
            if bovrd:  # the first operator in a pair of brackets
                op1, op2 = 0, 1
            else:
                # TODO: improve to decently handle built-in functions
                try:
                    op1 = strexpr.priority[refop if refop else self.reqexprop]
                except KeyError:
                    op1 = 0
                op2 = strexpr.priority[other]
            if op1 < op2:
                # the appending operator has higher priority
                assert self.isEnd()  # originally if
                # take the operator to the last appended operand
                self.req = plexpr(other, self.req, None)
                self.level += 1
            else:
                # the appeding operator has lower priotity
                if op2 < op1 and not bsovrd and self.level > reflevel + 1:
                    self.level -= 1
                assert self.isEnd()
                self.reqexpr = plexpr(other, copy(self.reqexpr), None)
        elif other in strexpr.PREDEFFUNC:
            self.req = plexpr(other, self.req, None)
            self.level += 1
        elif other[0].isalpha():
            self.req = other
        elif other not in strexpr.brackets:  # number detected
            self.req = int(other) if other.isnumeric() else float(other)

    @classmethod
    # TODO: to be pass-by-reference or value?
    def simplify(cls, expr, permissive=False):
        for L in range(get_level(expr), 0, -1):
            bottoms = find_level(expr, L)
            bottoms = {_cur[:-1] for _cur in bottoms}
            for i in bottoms:
                # TODO: division as inverse of multiplication
                subexpr = expr.get(i)
                typeset = {type(subexpr[1]), type(subexpr[2])}
                if typeset <= {int, float}:
                    expr.set(i, subexpr())
                elif type(None) not in typeset:  # not a predeffunc
                    for RR in RR_DICT[subexpr.stroperator]:
                        if RR.verify(subexpr):
                            expr.set(i, RR.rewrite(subexpr))
                            break
                        for alt in strexpr.OP_MAP[subexpr.stroperator].alt:
                            if RR.verify(alt(subexpr)):
                                expr.set(i, RR.rewrite(subexpr))
                    if permissive:
                        pass
        return expr

    def __add__(self, other):
        return plexpr('+', self, other)

    def __sub__(self, other):
        return plexpr('-', self, other)

    def __iter__(self):
        for i in (1, 2):
            yield self[i]

    def __getitem__(self, index):
        if index is 2:
            return self.arg2
        elif index is 1:
            return self.arg1
        return self.stroperator

    def __setitem__(self, index, value):
        if index is 1:
            self.arg1 = value
        elif index is 2:
            self.arg2 = value
        else:
            self.stroperator = value

    def __repr__(self):
        return plexpr.output.substitute(op=self.stroperator.rstrip('('),
                                        arg1=self.arg1
                                        if self.arg1 is not None else '',
                                        arg2=self.arg2)

    def __call__(self, params={}):
        op1, op2 = self.arg1, self.arg2
        if type(op1) is plexpr:
            op1 = op1(params)
        elif type(op1) is str:
            try:
                op1 = params[op1]
            except KeyError:
                raise UnspecifiedVariable
        if type(op2) is plexpr:
            op2 = op2(params)
        elif type(op2) is str:
            try:
                op2 = params[op2]
            except KeyError:
                raise UnspecifiedVariable
        if op1 is not None:
            return self.operate(op1, op2)
        return self.operate(op2)


class strexpr:
    """
    A unique sturcture (to be cythonized) for math
    expressions which converts a string to polish
    notation stored in its final attribute.
    Supports four elementary arithmetic operations and
    trigonometric, hyperbolic and logarithm functions.

    """
    PREDEFFUNC = {'sin', 'cos', 'tan',
                  'csc', 'sec', 'cot',
                  'sinh', 'cosh', 'tanh',
                  'ln', 'sqrt'}
    Lbrackets = {'(', '[', '{'}
    Rbrackets = {')', ']', '{'}
    brackets = Lbrackets | Rbrackets
    # TODO: add space support between vars (re?)
    operators = {'-', '+', '*', '/', '^', ' '}
    OP_MAP = {'+': OP_PLUS, '*': OP_MUL}
    priority = dict(zip(['-', '+', '*', ' ', '/', '^'],
                        [1, 1, 2, 2, 2, 3]))
    tokens = operators | brackets

    ACtrie = ACProcessor(tokens, reduced=True)

    def __init__(self, _str, params=None):
        self.str = _str
        self.expr = None
        self.params = params
        self.requesting = []
        self.final = plexpr('+', 0, None)
        self.process(self.preprocess())
        # should mark innermost level here
#         print(self.final)

    def preprocess(self):
        counter = strexpr.ACtrie(self.str)
        self.expr = strexpr.ACtrie.record

        if sum(counter[i] for i in strexpr.Lbrackets) is not counter[')']:
            raise SyntaxError('Inconsistent brackets')

        self.cut()

        # generate token mapping (mark token priority with their position)
        _T = {i[0]: strexpr.priority.get(i[1], 4)
              for i in enumerate(self.tokenchain)
              if i[1] in strexpr.operators | strexpr.PREDEFFUNC}
        # sort _T by key
        _O = odict(sorted(_T.items()))
        # process the raw mapping (naive method)
        # find adjacent redirect of backspace as to verify permission
        _R = {}
        for i in reversed(_O):
            for j in reversed(_O):
                if j < i and _O[j] is _O[i]:
                    _R[i] = j
        return _T, _O, _R

    def cut(self):
        tokens = [pos for key in self.expr.values() for pos in key]
        tokens.extend([pos + len(key)
                       for key in self.expr
                       for pos in self.expr[key]])
        tokens.sort()
        tokens = [0] + tokens + [len(self.str)]
        self.tokenchain = [self.str[x:y]
                           for x, y in zip(tokens, tokens[1:]) if y != x]

    def bsovrdperm(self, index, _T, _O):
        for i in range(_O[index], index):
            if i in _O and _T[i] < _T[index]:
                return True
        return False

    def process(self, TORtuple):
        # TODO: rename _T,_O,_R to improve readablity
        _T, _O, _R = TORtuple
        _list = self.tokenchain
        # preprocess not well-behaved strings
        if _list[0] in ('-', '+'):
            self.process(['0'] + self.cut())
        else:  # main process goes here
            refop = []
            reflevel = []
            opreq = False  # mark to request the first operator in brackets
            bsovrd = False
            for index, token in enumerate(_list):
                # TODO: to accept something like tan^2(x)
                if opreq and token in strexpr.operators:
                    opreq = 2
                if token is '(':
                    refop.append(self.final.reqexprop)
                    reflevel.append(self.final.level)
                    opreq = True
                elif token[-1] is '(':
                    # built-in func detected
                    # built-in func itself create a level in plexpr
                    # add another level in reflevel
                    refop.append(self.final.reqexprop)
                    reflevel.append(self.final.level + 2)
                    opreq = True
                # handle backspace exception
                if index not in _R or self.bsovrdperm(index, _T, _O):
                    bsovrd = True
                self.final.append(token,
                                  refop.pop(-1) if refop else None,
                                  opreq,
                                  reflevel[-1] if reflevel else [],
                                  bsovrd)

                if bsovrd:
                    bsovrd = False
                if opreq is 2:
                    opreq = False
                if token in strexpr.operators:
                    refop.append(token)
                if token is ')':
                    self.final.level = reflevel.pop(-1)
#             self.final = plexpr.simplify(self.final)

    def __call__(self, valuedict={}):
        valuedict.update({'Pi': math.pi, 'e': math.e})
        return self.final(valuedict)

    def latex(self):
        return

    def __repr__(self):
        return str(self.final)


class relexpr():

    def __init__(self, name, _str, values={}):
        try:
            self.l, self.r = _str.split('=')
        except:
            raise MathError('Invalid equation')
        self.l = strexpr(self.l, values)
        self.r = strexpr(self.r, values)
        self.values = values

    def setvalues(self, valuedict: dict):
        self.values = valuedict

    def simplify(self):
        pass

    def __bool__(self):
        return self.l(self.values) == self.r(self.values)


def get_level(expr: plexpr):
    string = repr(expr)  # reverse a plexpr to usual expression
    blevel = 0
    result = {}
    for index, i in enumerate(string):
        if i is '(':
            blevel += 1
        elif i is ')':
            blevel -= 1
        result[index] = blevel
    return max(result.values())


def find_level(expr: plexpr, level: int):
    # enumerate!
    possibilities = cartesian((1, 2), repeat=level)
    for i in possibilities:
        try:
            expr.get(i)
            yield i
        except (TypeError, IndexError):  # IndexError for symbolic variables
            pass

if __name__ == '__main__':
#     from math import isclose
    while True:
        try:
            expr = input('>>> ')
            se = strexpr(expr)
            print(se.final)
            plexpr.simplify(se.final)
#             print(isclose(se(), eval(expr.replace('^', '**'))), se.final)
            print(se.final)
        except SyntaxError:
            print('Unbalanced Brackets')

"""Generic Math Processing
"""
import AhoCorasick


class expr():
    """
    A unique container (data structure to be written in
    C++) for math expression.
    Supports four elementary arithmetic operations and
    trigonometric and hyperbolic functions.

    """

    predeffunc = {'sin', 'cos', 'tan',
                  'csc', 'sec', 'cot',
                  'sinh', 'cosh', 'tanh',
                  'ln'}
    predefconst = {'e', 'pi'}
    # predeffunc = {_func + '(' for _func in predeffunc}
    pmlevel = {'+', '-'}
    pdlevel = {'*', '/'}
    powlevel = {'^'}
    brackets = {'(', ')'}
    operators = pmlevel | pdlevel | powlevel | brackets
    tokens = operators | predeffunc | predefconst

    ACtrie = AhoCorasick.ACtrie(tokens)

    def __init__(self, _str, params=None):
        self.str = _str
        self.pos = 0
        self.expr = None
        self.params = params
        self.preprocess()

    def preprocess(self):
        _str = self.str
        expr.ACtrie.process(self.str)
        self.expr = expr.ACtrie.record
        try:
            assert len(self.expr['(']) == len(self.expr[')'])
        except AssertionError:
            raise SyntaxError('Unbalanced brackets')
        tokendict = dict((i, k) for k, v in self.expr.items() for i in v)
        cursor = min(tokendict)

        while cursor < len(_str):  # process span tokens
            if cursor in tokendict:
                token = tokendict[cursor]
                if token in expr.brackets:
                    brackets = self.matchbrackets(cursor)
                    print(_str[brackets[0]:brackets[-1] + 1])
                    cursor = brackets[-1]
                    continue
                elif token in expr.predeffunc:
                    brackets = self.matchbrackets(cursor)
                    print(token, _str[brackets[0]:brackets[-1] + 1])
                    cursor = brackets[-1]
                    continue
            cursor += 1

    def matchbrackets(self, cursor):
        _str = self.str
        bracket = []
        subcur = cursor
        while subcur < len(_str):
            if _str[subcur] == '(':
                break
            subcur += 1
        bracket.append(subcur)
        subcur += 1
        while subcur < len(_str):
            if _str[subcur] == ')':
                break
            elif _str[subcur] == '(':
                subbracket = self.matchbrackets(subcur)
                bracket.append(subbracket)
                subcur = subbracket[-1] + 1
                continue
            subcur += 1
        bracket.append(subcur)
        return bracket

    def __call__(self, valuelist):
        return

    def advance(self):
        self.pos += 1
        return self.str[self.pos]

    def latex(self):
        return

    def __repr__(self):
        return str(self.expr)


class eqn():

    def __init__(self, _str, values=None, params=None):
        self.l, self.r = _str.split('=')
        self.l = expr(self.l, params)
        self.r = expr(self.r, params)
        self.values = values

    def __bool__(self):
        return self.l(self.values) == self.r(self.values)

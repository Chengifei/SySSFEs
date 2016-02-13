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
                  'exp', 'ln'}
    _keywords = {'+', '-', '*', '/', '(', ')'} | predeffunc
    # ACtrie = AhoCorasick.ACtrie(_keywords)

    def __init__(self, _str, params=None):
        self.str = _str
        self.pos = 0
        self.expr = None
        self.params = params

    def preprocess(self):
        # self.expr = expr.ACtrie.process(self.str)
        pass

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

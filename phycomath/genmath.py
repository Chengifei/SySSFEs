'''
Created on Feb 10, 2016

@author: yfzheng
'''
"""
Generic Math Processing
"""
class expr():
    """
    :class: expr
    A unique container (data structure to be
    written in C++) for math expression.

    """

    predeffunc = {'sin', 'cos', 'tan',
                  'csc', 'sec', 'cot',
                  'sinh', 'cosh', 'tanh'}

    def __init__(self, str):
        self.str = str
        self.pos = 0
        self.expr = None

    def preprocess(self):

        _expr = {}
        curlevel = 0
        for i in str:
            pass
        self.expr = _expr

    def __call__(self, valuelist):
        return

    def advance(self):
        self.pos += 1
        return self.str[self.pos]

    def latex(self):
        return

    def __repr__(self):
        return str(self.expr)

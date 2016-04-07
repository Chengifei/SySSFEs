'''
Created on Feb 9, 2016

@author: yfzheng
'''
from copy import deepcopy as copy
from phycomath.genmath import strexpr, plexpr
from phyco.error import UnderConstruction


def diff(expr: plexpr, var: str):

    # these should be contained in a class
    def dadd(expr, var):
        return plexpr('+', diff(expr[1], var), diff(expr[2], var))

    def dsub(expr, var):
        return plexpr('-', diff(expr[1], var), diff(expr[2], var))

    def dmul(expr, var):
        return plexpr('+', plexpr('*', diff(expr[1], var), expr[2]),
                      plexpr('*', diff(expr[2], var), expr[1]))

    def ddiv(expr, var):
        return plexpr('/',
                      plexpr('-',
                             plexpr('*', diff(expr[1], var), expr[2]),
                             plexpr('*', diff(expr[2], var), expr[1])),
                      plexpr('*', expr[2], expr[2]))

    def dpow(expr, var):
        return plexpr('*',
                      copy(expr),
                      plexpr('+',
                             plexpr('*',
                                    diff(expr[2], var),
                                    plexpr('ln(', None, expr[1])),
                             plexpr('/',
                                    plexpr('*',
                                           expr[2],
                                           diff(expr[1], var)),
                                    expr[1]
                                    )
                             ))

    def dln(expr, var):
        return plexpr('/', diff(expr[2], var), copy(expr))

    corrRule = {'+': dadd, '-': dsub, '*': dmul, '/': ddiv, '^': dpow,
                'ln(': dln}

    if type(expr) is plexpr:
        try:
            return corrRule[expr.stroperator](expr, var)
        except KeyError:
            raise UnderConstruction
    elif expr is var:
        return 1
    else:
        return 0


if __name__ == '__main__':
    print(plexpr.simplify((diff(strexpr(input('d/dx ')).final, 'x'))))

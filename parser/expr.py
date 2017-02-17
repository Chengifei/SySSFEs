#!/usr/bin/python
import sys
import os
from antlr4 import ParseTreeVisitor, InputStream, CommonTokenStream

if __name__ != '__main__':
    from .exprLexer import exprLexer
    from .exprParser import exprParser
    from .exprVisitor import exprVisitor
else:
    from exprLexer import exprLexer
    from exprParser import exprParser
    from exprVisitor import exprVisitor


class Op:
    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return 'Op:' + self.name

    def __eq__(self, other):
        return self.name == other.name


class Variable:
    def __init__(self, name):
        self.name = name


class exprVisitor(ParseTreeVisitor):
    # Visit a parse tree produced by exprParser#expression.
    def visitExpression(self, ctx: exprParser.ExpressionContext):
        result = (None, [])
        lastop = None
        for i in ctx.children:
            thisresult = i.accept(self)
            if thisresult is not None:
                result[1].append(thisresult)
            elif lastop != i.getText():
                op = i.getText()
                result = (op, [result] if len(result[1]) > 1 else result[1])
                lastop = op
        return result if len(result[1]) != 1 else result[1][0]

    # Visit a parse tree produced by exprParser#multiplyingExpression.
    def visitMultiplyingExpression(self, ctx: exprParser.MultiplyingExpressionContext):
        result = (None, [])
        lastop = None
        for i in ctx.children:
            thisresult = i.accept(self)
            if thisresult is not None:
                result[1].append(thisresult)
            elif lastop != i.getText():
                op = i.getText()
                result = (op, [result] if len(result[1]) > 1 else result[1])
                lastop = op
        return result if len(result[1]) != 1 else result[1][0]

    # Visit a parse tree produced by exprParser#powExpression.
    def visitPowExpression(self, ctx: exprParser.PowExpressionContext):
        result = ('pow', [])
        for i in ctx.children:
            thisresult = i.accept(self)
            if thisresult is not None:
                result[1].append(thisresult)
        return result if len(result[1]) != 1 else result[1][0]

    # Visit a parse tree produced by exprParser#atom.
    def visitAtom(self, ctx: exprParser.AtomContext):
        for i in ctx.children:
            thisresult = i.accept(self)
            if thisresult is not None:
                result = thisresult
        return result

    # Visit a parse tree produced by exprParser#func.
    def visitFunc(self, ctx: exprParser.FuncContext):
        ctx.removeLastChild()
        result = []
        func = ''
        for i in ctx.children:
            thisresult = i.accept(self)
            if thisresult:
                result.append(thisresult)
            elif i.symbol.text:
                func += i.symbol.text
        func = func[:-1]
        return func, result

    # Visit a parse tree produced by exprParser#number.
    def visitNumber(self, ctx: exprParser.NumberContext):
        text = ctx.getText()
        return float(text) if '.' in text else int(text)

    # Visit a parse tree produced by exprParser#variable.
    def visitVariable(self, ctx: exprParser.VariableContext):
        return ctx.getText()


def flatten(expression: tuple):
    if isinstance(expression, tuple):
        yield Op(expression[0])
        for i in expression[1]:
            yield from flatten(i)
        yield None
    else:
        yield expression


def stringToExpr(string):
    input = InputStream(string)
    sys.stdout = open(os.devnull, 'w')
    stream = CommonTokenStream(exprLexer(input))
    parsetree = exprParser(stream)
    sys.stdout = sys.__stdout__
    tree = parsetree.expression()
    return exprVisitor().visit(tree)


def postorder(expression: tuple):
    if isinstance(expression, tuple):
        for i in expression[1][:2]:
            yield from postorder(i)
        for i in expression[1][2:]:
            yield Op(expression[0])
            yield from postorder(i)
        yield Op(expression[0])
    else:
        yield expression


def preorder(expression: tuple):
    if isinstance(expression, tuple):
        yield Op(expression[0])
        for i in expression[1][:2]:
            yield from preorder(i)
        for i in expression[1][2:]:
            yield Op(expression[0])
            yield from preorder(i)
    else:
        yield expression


if __name__ == '__main__':
    expr = stringToExpr(input())
    print(list(flatten(expr)))

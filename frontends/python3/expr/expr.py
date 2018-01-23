#!/usr/bin/python
# Copyright 2017-2018 by Yifei Zheng
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import os
from antlr4 import ParseTreeVisitor, InputStream, CommonTokenStream

# if __name__ != '__main__':
from .exprLexer import exprLexer
from .exprParser import exprParser
from .exprVisitor import exprVisitor


# else:
#     from exprLexer import exprLexer
#     from exprParser import exprParser
#     from exprVisitor import exprVisitor


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


def stringToExpr(string):
    input = InputStream(string)
    sys.stdout = open(os.devnull, 'w')
    stream = CommonTokenStream(exprLexer(input))
    parsetree = exprParser(stream)
    sys.stdout = sys.__stdout__
    tree = parsetree.expression()
    return exprVisitor().visit(tree)


if __name__ == '__main__':
    expr = stringToExpr(input())
    print(expr)

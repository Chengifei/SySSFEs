# Generated from expr.g4 by ANTLR 4.6
from antlr4 import *
from .exprParser import exprParser

# This class defines a complete generic visitor for a parse tree produced by exprParser.

class exprVisitor(ParseTreeVisitor):

    # Visit a parse tree produced by exprParser#expression.
    def visitExpression(self, ctx:exprParser.ExpressionContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by exprParser#multiplyingExpression.
    def visitMultiplyingExpression(self, ctx:exprParser.MultiplyingExpressionContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by exprParser#powExpression.
    def visitPowExpression(self, ctx:exprParser.PowExpressionContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by exprParser#atom.
    def visitAtom(self, ctx:exprParser.AtomContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by exprParser#func.
    def visitFunc(self, ctx:exprParser.FuncContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by exprParser#number.
    def visitNumber(self, ctx:exprParser.NumberContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by exprParser#variable.
    def visitVariable(self, ctx:exprParser.VariableContext):
        return self.visitChildren(ctx)



del exprParser

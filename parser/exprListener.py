# Generated from expr.g4 by ANTLR 4.6
from antlr4 import *
if __name__ is not None and "." in __name__:
    from .exprParser import exprParser
else:
    from exprParser import exprParser

# This class defines a complete listener for a parse tree produced by exprParser.
class exprListener(ParseTreeListener):

    # Enter a parse tree produced by exprParser#expression.
    def enterExpression(self, ctx:exprParser.ExpressionContext):
        pass

    # Exit a parse tree produced by exprParser#expression.
    def exitExpression(self, ctx:exprParser.ExpressionContext):
        pass


    # Enter a parse tree produced by exprParser#multiplyingExpression.
    def enterMultiplyingExpression(self, ctx:exprParser.MultiplyingExpressionContext):
        pass

    # Exit a parse tree produced by exprParser#multiplyingExpression.
    def exitMultiplyingExpression(self, ctx:exprParser.MultiplyingExpressionContext):
        pass


    # Enter a parse tree produced by exprParser#powExpression.
    def enterPowExpression(self, ctx:exprParser.PowExpressionContext):
        pass

    # Exit a parse tree produced by exprParser#powExpression.
    def exitPowExpression(self, ctx:exprParser.PowExpressionContext):
        pass


    # Enter a parse tree produced by exprParser#atom.
    def enterAtom(self, ctx:exprParser.AtomContext):
        pass

    # Exit a parse tree produced by exprParser#atom.
    def exitAtom(self, ctx:exprParser.AtomContext):
        pass


    # Enter a parse tree produced by exprParser#func.
    def enterFunc(self, ctx:exprParser.FuncContext):
        pass

    # Exit a parse tree produced by exprParser#func.
    def exitFunc(self, ctx:exprParser.FuncContext):
        pass


    # Enter a parse tree produced by exprParser#number.
    def enterNumber(self, ctx:exprParser.NumberContext):
        pass

    # Exit a parse tree produced by exprParser#number.
    def exitNumber(self, ctx:exprParser.NumberContext):
        pass


    # Enter a parse tree produced by exprParser#variable.
    def enterVariable(self, ctx:exprParser.VariableContext):
        pass

    # Exit a parse tree produced by exprParser#variable.
    def exitVariable(self, ctx:exprParser.VariableContext):
        pass



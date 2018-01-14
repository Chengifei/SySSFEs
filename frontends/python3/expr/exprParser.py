# Generated from expr.g4 by ANTLR 4.6
# encoding: utf-8
from antlr4 import *
from io import StringIO

def serializedATN():
    with StringIO() as buf:
        buf.write("\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\3\17")
        buf.write("\\\4\2\t\2\4\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b")
        buf.write("\t\b\3\2\3\2\3\2\7\2\24\n\2\f\2\16\2\27\13\2\3\3\3\3\3")
        buf.write("\3\7\3\34\n\3\f\3\16\3\37\13\3\3\4\3\4\3\4\5\4$\n\4\3")
        buf.write("\5\3\5\3\5\3\5\3\5\3\5\3\5\5\5-\n\5\3\6\7\6\60\n\6\f\6")
        buf.write("\16\6\63\13\6\3\6\3\6\3\6\3\6\3\6\7\6:\n\6\f\6\16\6=\13")
        buf.write("\6\3\6\3\6\3\7\5\7B\n\7\3\7\3\7\3\7\5\7G\n\7\3\b\5\bJ")
        buf.write("\n\b\3\b\3\b\7\bN\n\b\f\b\16\bQ\13\b\3\b\7\bT\n\b\f\b")
        buf.write("\16\bW\13\b\3\b\5\bZ\n\b\3\b\2\2\t\2\4\6\b\n\f\16\2\5")
        buf.write("\3\2\7\b\3\2\t\n\3\2\f\rb\2\20\3\2\2\2\4\30\3\2\2\2\6")
        buf.write(" \3\2\2\2\b,\3\2\2\2\n\61\3\2\2\2\fA\3\2\2\2\16I\3\2\2")
        buf.write("\2\20\25\5\4\3\2\21\22\t\2\2\2\22\24\5\4\3\2\23\21\3\2")
        buf.write("\2\2\24\27\3\2\2\2\25\23\3\2\2\2\25\26\3\2\2\2\26\3\3")
        buf.write("\2\2\2\27\25\3\2\2\2\30\35\5\6\4\2\31\32\t\3\2\2\32\34")
        buf.write("\5\6\4\2\33\31\3\2\2\2\34\37\3\2\2\2\35\33\3\2\2\2\35")
        buf.write("\36\3\2\2\2\36\5\3\2\2\2\37\35\3\2\2\2 #\5\b\5\2!\"\7")
        buf.write("\3\2\2\"$\5\b\5\2#!\3\2\2\2#$\3\2\2\2$\7\3\2\2\2%-\5\16")
        buf.write("\b\2&-\5\f\7\2\'(\7\5\2\2()\5\2\2\2)*\7\6\2\2*-\3\2\2")
        buf.write("\2+-\5\n\6\2,%\3\2\2\2,&\3\2\2\2,\'\3\2\2\2,+\3\2\2\2")
        buf.write("-\t\3\2\2\2.\60\7\16\2\2/.\3\2\2\2\60\63\3\2\2\2\61/\3")
        buf.write("\2\2\2\61\62\3\2\2\2\62\64\3\2\2\2\63\61\3\2\2\2\64\65")
        buf.write("\7\f\2\2\65\66\7\5\2\2\66;\5\2\2\2\678\7\4\2\28:\5\2\2")
        buf.write("\29\67\3\2\2\2:=\3\2\2\2;9\3\2\2\2;<\3\2\2\2<>\3\2\2\2")
        buf.write("=;\3\2\2\2>?\7\6\2\2?\13\3\2\2\2@B\7\b\2\2A@\3\2\2\2A")
        buf.write("B\3\2\2\2BC\3\2\2\2CF\7\r\2\2DE\7\13\2\2EG\7\r\2\2FD\3")
        buf.write("\2\2\2FG\3\2\2\2G\r\3\2\2\2HJ\7\b\2\2IH\3\2\2\2IJ\3\2")
        buf.write("\2\2JY\3\2\2\2KO\7\f\2\2LN\t\4\2\2ML\3\2\2\2NQ\3\2\2\2")
        buf.write("OM\3\2\2\2OP\3\2\2\2PZ\3\2\2\2QO\3\2\2\2RT\7\16\2\2SR")
        buf.write("\3\2\2\2TW\3\2\2\2US\3\2\2\2UV\3\2\2\2VX\3\2\2\2WU\3\2")
        buf.write("\2\2XZ\7\f\2\2YK\3\2\2\2YU\3\2\2\2Z\17\3\2\2\2\16\25\35")
        buf.write("#,\61;AFIOUY")
        return buf.getvalue()


class exprParser ( Parser ):

    grammarFileName = "expr.g4"

    atn = ATNDeserializer().deserialize(serializedATN())

    decisionsToDFA = [ DFA(ds, i) for i, ds in enumerate(atn.decisionToState) ]

    sharedContextCache = PredictionContextCache()

    literalNames = [ "<INVALID>", "'^'", "','", "'('", "')'", "'+'", "'-'", 
                     "'*'", "'/'", "'.'" ]

    symbolicNames = [ "<INVALID>", "<INVALID>", "<INVALID>", "LPAREN", "RPAREN", 
                      "PLUS", "MINUS", "TIMES", "DIV", "POINT", "LETTERS", 
                      "DIGITS", "MEMBER", "WS" ]

    RULE_expression = 0
    RULE_multiplyingExpression = 1
    RULE_powExpression = 2
    RULE_atom = 3
    RULE_func = 4
    RULE_number = 5
    RULE_variable = 6

    ruleNames =  [ "expression", "multiplyingExpression", "powExpression", 
                   "atom", "func", "number", "variable" ]

    EOF = Token.EOF
    T__0=1
    T__1=2
    LPAREN=3
    RPAREN=4
    PLUS=5
    MINUS=6
    TIMES=7
    DIV=8
    POINT=9
    LETTERS=10
    DIGITS=11
    MEMBER=12
    WS=13

    def __init__(self, input:TokenStream):
        super().__init__(input)
        self.checkVersion("4.6")
        self._interp = ParserATNSimulator(self, self.atn, self.decisionsToDFA, self.sharedContextCache)
        self._predicates = None



    class ExpressionContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def multiplyingExpression(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(exprParser.MultiplyingExpressionContext)
            else:
                return self.getTypedRuleContext(exprParser.MultiplyingExpressionContext,i)


        def PLUS(self, i:int=None):
            if i is None:
                return self.getTokens(exprParser.PLUS)
            else:
                return self.getToken(exprParser.PLUS, i)

        def MINUS(self, i:int=None):
            if i is None:
                return self.getTokens(exprParser.MINUS)
            else:
                return self.getToken(exprParser.MINUS, i)

        def getRuleIndex(self):
            return exprParser.RULE_expression

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterExpression" ):
                listener.enterExpression(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitExpression" ):
                listener.exitExpression(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitExpression" ):
                return visitor.visitExpression(self)
            else:
                return visitor.visitChildren(self)




    def expression(self):

        localctx = exprParser.ExpressionContext(self, self._ctx, self.state)
        self.enterRule(localctx, 0, self.RULE_expression)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 14
            self.multiplyingExpression()
            self.state = 19
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==exprParser.PLUS or _la==exprParser.MINUS:
                self.state = 15
                _la = self._input.LA(1)
                if not(_la==exprParser.PLUS or _la==exprParser.MINUS):
                    self._errHandler.recoverInline(self)
                else:
                    self._errHandler.reportMatch(self)
                    self.consume()
                self.state = 16
                self.multiplyingExpression()
                self.state = 21
                self._errHandler.sync(self)
                _la = self._input.LA(1)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class MultiplyingExpressionContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def powExpression(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(exprParser.PowExpressionContext)
            else:
                return self.getTypedRuleContext(exprParser.PowExpressionContext,i)


        def TIMES(self, i:int=None):
            if i is None:
                return self.getTokens(exprParser.TIMES)
            else:
                return self.getToken(exprParser.TIMES, i)

        def DIV(self, i:int=None):
            if i is None:
                return self.getTokens(exprParser.DIV)
            else:
                return self.getToken(exprParser.DIV, i)

        def getRuleIndex(self):
            return exprParser.RULE_multiplyingExpression

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterMultiplyingExpression" ):
                listener.enterMultiplyingExpression(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitMultiplyingExpression" ):
                listener.exitMultiplyingExpression(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitMultiplyingExpression" ):
                return visitor.visitMultiplyingExpression(self)
            else:
                return visitor.visitChildren(self)




    def multiplyingExpression(self):

        localctx = exprParser.MultiplyingExpressionContext(self, self._ctx, self.state)
        self.enterRule(localctx, 2, self.RULE_multiplyingExpression)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 22
            self.powExpression()
            self.state = 27
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==exprParser.TIMES or _la==exprParser.DIV:
                self.state = 23
                _la = self._input.LA(1)
                if not(_la==exprParser.TIMES or _la==exprParser.DIV):
                    self._errHandler.recoverInline(self)
                else:
                    self._errHandler.reportMatch(self)
                    self.consume()
                self.state = 24
                self.powExpression()
                self.state = 29
                self._errHandler.sync(self)
                _la = self._input.LA(1)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class PowExpressionContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def atom(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(exprParser.AtomContext)
            else:
                return self.getTypedRuleContext(exprParser.AtomContext,i)


        def getRuleIndex(self):
            return exprParser.RULE_powExpression

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterPowExpression" ):
                listener.enterPowExpression(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitPowExpression" ):
                listener.exitPowExpression(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitPowExpression" ):
                return visitor.visitPowExpression(self)
            else:
                return visitor.visitChildren(self)




    def powExpression(self):

        localctx = exprParser.PowExpressionContext(self, self._ctx, self.state)
        self.enterRule(localctx, 4, self.RULE_powExpression)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 30
            self.atom()
            self.state = 33
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            if _la==exprParser.T__0:
                self.state = 31
                self.match(exprParser.T__0)
                self.state = 32
                self.atom()


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class AtomContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def variable(self):
            return self.getTypedRuleContext(exprParser.VariableContext,0)


        def number(self):
            return self.getTypedRuleContext(exprParser.NumberContext,0)


        def LPAREN(self):
            return self.getToken(exprParser.LPAREN, 0)

        def expression(self):
            return self.getTypedRuleContext(exprParser.ExpressionContext,0)


        def RPAREN(self):
            return self.getToken(exprParser.RPAREN, 0)

        def func(self):
            return self.getTypedRuleContext(exprParser.FuncContext,0)


        def getRuleIndex(self):
            return exprParser.RULE_atom

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterAtom" ):
                listener.enterAtom(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitAtom" ):
                listener.exitAtom(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitAtom" ):
                return visitor.visitAtom(self)
            else:
                return visitor.visitChildren(self)




    def atom(self):

        localctx = exprParser.AtomContext(self, self._ctx, self.state)
        self.enterRule(localctx, 6, self.RULE_atom)
        try:
            self.state = 42
            self._errHandler.sync(self)
            la_ = self._interp.adaptivePredict(self._input,3,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 35
                self.variable()
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 36
                self.number()
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 37
                self.match(exprParser.LPAREN)
                self.state = 38
                self.expression()
                self.state = 39
                self.match(exprParser.RPAREN)
                pass

            elif la_ == 4:
                self.enterOuterAlt(localctx, 4)
                self.state = 41
                self.func()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class FuncContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def LETTERS(self):
            return self.getToken(exprParser.LETTERS, 0)

        def LPAREN(self):
            return self.getToken(exprParser.LPAREN, 0)

        def expression(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(exprParser.ExpressionContext)
            else:
                return self.getTypedRuleContext(exprParser.ExpressionContext,i)


        def RPAREN(self):
            return self.getToken(exprParser.RPAREN, 0)

        def MEMBER(self, i:int=None):
            if i is None:
                return self.getTokens(exprParser.MEMBER)
            else:
                return self.getToken(exprParser.MEMBER, i)

        def getRuleIndex(self):
            return exprParser.RULE_func

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterFunc" ):
                listener.enterFunc(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitFunc" ):
                listener.exitFunc(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitFunc" ):
                return visitor.visitFunc(self)
            else:
                return visitor.visitChildren(self)




    def func(self):

        localctx = exprParser.FuncContext(self, self._ctx, self.state)
        self.enterRule(localctx, 8, self.RULE_func)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 47
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==exprParser.MEMBER:
                self.state = 44
                self.match(exprParser.MEMBER)
                self.state = 49
                self._errHandler.sync(self)
                _la = self._input.LA(1)

            self.state = 50
            self.match(exprParser.LETTERS)
            self.state = 51
            self.match(exprParser.LPAREN)
            self.state = 52
            self.expression()
            self.state = 57
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==exprParser.T__1:
                self.state = 53
                self.match(exprParser.T__1)
                self.state = 54
                self.expression()
                self.state = 59
                self._errHandler.sync(self)
                _la = self._input.LA(1)

            self.state = 60
            self.match(exprParser.RPAREN)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class NumberContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def DIGITS(self, i:int=None):
            if i is None:
                return self.getTokens(exprParser.DIGITS)
            else:
                return self.getToken(exprParser.DIGITS, i)

        def MINUS(self):
            return self.getToken(exprParser.MINUS, 0)

        def POINT(self):
            return self.getToken(exprParser.POINT, 0)

        def getRuleIndex(self):
            return exprParser.RULE_number

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterNumber" ):
                listener.enterNumber(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitNumber" ):
                listener.exitNumber(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitNumber" ):
                return visitor.visitNumber(self)
            else:
                return visitor.visitChildren(self)




    def number(self):

        localctx = exprParser.NumberContext(self, self._ctx, self.state)
        self.enterRule(localctx, 10, self.RULE_number)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 63
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            if _la==exprParser.MINUS:
                self.state = 62
                self.match(exprParser.MINUS)


            self.state = 65
            self.match(exprParser.DIGITS)
            self.state = 68
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            if _la==exprParser.POINT:
                self.state = 66
                self.match(exprParser.POINT)
                self.state = 67
                self.match(exprParser.DIGITS)


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class VariableContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def LETTERS(self, i:int=None):
            if i is None:
                return self.getTokens(exprParser.LETTERS)
            else:
                return self.getToken(exprParser.LETTERS, i)

        def MINUS(self):
            return self.getToken(exprParser.MINUS, 0)

        def MEMBER(self, i:int=None):
            if i is None:
                return self.getTokens(exprParser.MEMBER)
            else:
                return self.getToken(exprParser.MEMBER, i)

        def DIGITS(self, i:int=None):
            if i is None:
                return self.getTokens(exprParser.DIGITS)
            else:
                return self.getToken(exprParser.DIGITS, i)

        def getRuleIndex(self):
            return exprParser.RULE_variable

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterVariable" ):
                listener.enterVariable(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitVariable" ):
                listener.exitVariable(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitVariable" ):
                return visitor.visitVariable(self)
            else:
                return visitor.visitChildren(self)




    def variable(self):

        localctx = exprParser.VariableContext(self, self._ctx, self.state)
        self.enterRule(localctx, 12, self.RULE_variable)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 71
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            if _la==exprParser.MINUS:
                self.state = 70
                self.match(exprParser.MINUS)


            self.state = 87
            self._errHandler.sync(self)
            la_ = self._interp.adaptivePredict(self._input,11,self._ctx)
            if la_ == 1:
                self.state = 73
                self.match(exprParser.LETTERS)
                self.state = 77
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==exprParser.LETTERS or _la==exprParser.DIGITS:
                    self.state = 74
                    _la = self._input.LA(1)
                    if not(_la==exprParser.LETTERS or _la==exprParser.DIGITS):
                        self._errHandler.recoverInline(self)
                    else:
                        self._errHandler.reportMatch(self)
                        self.consume()
                    self.state = 79
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                pass

            elif la_ == 2:
                self.state = 83
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==exprParser.MEMBER:
                    self.state = 80
                    self.match(exprParser.MEMBER)
                    self.state = 85
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                self.state = 86
                self.match(exprParser.LETTERS)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx






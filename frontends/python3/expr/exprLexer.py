# Generated from expr.g4 by ANTLR 4.6
from antlr4 import *
from io import StringIO


def serializedATN():
    with StringIO() as buf:
        buf.write("\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\2\17")
        buf.write("R\b\1\4\2\t\2\4\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7")
        buf.write("\4\b\t\b\4\t\t\t\4\n\t\n\4\13\t\13\4\f\t\f\4\r\t\r\4\16")
        buf.write("\t\16\3\2\3\2\3\3\3\3\3\4\3\4\3\5\3\5\3\6\3\6\3\7\3\7")
        buf.write("\3\b\3\b\3\t\3\t\3\n\3\n\3\13\6\13\61\n\13\r\13\16\13")
        buf.write("\62\3\f\6\f\66\n\f\r\f\16\f\67\3\r\6\r;\n\r\r\r\16\r<")
        buf.write("\3\r\3\r\3\r\3\r\3\r\6\rD\n\r\r\r\16\rE\3\r\3\r\5\rJ\n")
        buf.write("\r\3\16\6\16M\n\16\r\16\16\16N\3\16\3\16\2\2\17\3\3\5")
        buf.write("\4\7\5\t\6\13\7\r\b\17\t\21\n\23\13\25\f\27\r\31\16\33")
        buf.write("\17\3\2\4\4\2C\\c|\5\2\13\f\17\17\"\"X\2\3\3\2\2\2\2\5")
        buf.write("\3\2\2\2\2\7\3\2\2\2\2\t\3\2\2\2\2\13\3\2\2\2\2\r\3\2")
        buf.write("\2\2\2\17\3\2\2\2\2\21\3\2\2\2\2\23\3\2\2\2\2\25\3\2\2")
        buf.write("\2\2\27\3\2\2\2\2\31\3\2\2\2\2\33\3\2\2\2\3\35\3\2\2\2")
        buf.write("\5\37\3\2\2\2\7!\3\2\2\2\t#\3\2\2\2\13%\3\2\2\2\r\'\3")
        buf.write("\2\2\2\17)\3\2\2\2\21+\3\2\2\2\23-\3\2\2\2\25\60\3\2\2")
        buf.write("\2\27\65\3\2\2\2\31I\3\2\2\2\33L\3\2\2\2\35\36\7`\2\2")
        buf.write("\36\4\3\2\2\2\37 \7.\2\2 \6\3\2\2\2!\"\7*\2\2\"\b\3\2")
        buf.write("\2\2#$\7+\2\2$\n\3\2\2\2%&\7-\2\2&\f\3\2\2\2\'(\7/\2\2")
        buf.write("(\16\3\2\2\2)*\7,\2\2*\20\3\2\2\2+,\7\61\2\2,\22\3\2\2")
        buf.write("\2-.\7\60\2\2.\24\3\2\2\2/\61\t\2\2\2\60/\3\2\2\2\61\62")
        buf.write("\3\2\2\2\62\60\3\2\2\2\62\63\3\2\2\2\63\26\3\2\2\2\64")
        buf.write("\66\4\62;\2\65\64\3\2\2\2\66\67\3\2\2\2\67\65\3\2\2\2")
        buf.write("\678\3\2\2\28\30\3\2\2\29;\5\25\13\2:9\3\2\2\2;<\3\2\2")
        buf.write("\2<:\3\2\2\2<=\3\2\2\2=>\3\2\2\2>?\5\23\n\2?J\3\2\2\2")
        buf.write("@C\7&\2\2AD\5\27\f\2BD\5\25\13\2CA\3\2\2\2CB\3\2\2\2D")
        buf.write("E\3\2\2\2EC\3\2\2\2EF\3\2\2\2FG\3\2\2\2GH\5\23\n\2HJ\3")
        buf.write("\2\2\2I:\3\2\2\2I@\3\2\2\2J\32\3\2\2\2KM\t\3\2\2LK\3\2")
        buf.write("\2\2MN\3\2\2\2NL\3\2\2\2NO\3\2\2\2OP\3\2\2\2PQ\b\16\2")
        buf.write("\2Q\34\3\2\2\2\13\2\60\62\67<CEIN\3\2\3\2")
        return buf.getvalue()


class exprLexer(Lexer):

    atn = ATNDeserializer().deserialize(serializedATN())

    decisionsToDFA = [ DFA(ds, i) for i, ds in enumerate(atn.decisionToState) ]


    T__0 = 1
    T__1 = 2
    LPAREN = 3
    RPAREN = 4
    PLUS = 5
    MINUS = 6
    TIMES = 7
    DIV = 8
    POINT = 9
    LETTERS = 10
    DIGITS = 11
    MEMBER = 12
    WS = 13

    modeNames = [ "DEFAULT_MODE" ]

    literalNames = [ "<INVALID>",
            "'^'", "','", "'('", "')'", "'+'", "'-'", "'*'", "'/'", "'.'" ]

    symbolicNames = [ "<INVALID>",
            "LPAREN", "RPAREN", "PLUS", "MINUS", "TIMES", "DIV", "POINT", 
            "LETTERS", "DIGITS", "MEMBER", "WS" ]

    ruleNames = [ "T__0", "T__1", "LPAREN", "RPAREN", "PLUS", "MINUS", "TIMES", 
                  "DIV", "POINT", "LETTERS", "DIGITS", "MEMBER", "WS" ]

    grammarFileName = "expr.g4"

    def __init__(self, input=None):
        super().__init__(input)
        self.checkVersion("4.6")
        self._interp = LexerATNSimulator(self, self.atn, self.decisionsToDFA, PredictionContextCache())
        self._actions = None
        self._predicates = None



grammar expr;

unit
    :   Identifier
    |   combIdent
    |   memIdent
    |   number
    |   '(' expression ')'
    |   functionExpression
    ;

combIdent
    :   '$' Integer '.' Identifier
    ;

memIdent
    :   '$' Identifier '.' Identifier
    ;

functionExpression
    :   Identifier '(' expression (',' expression)* ')'
    ;

powExpression
    :   unit
    |   powExpression '^' unit
    ;

multiplicativeExpression
    :   powExpression
    |   multiplicativeExpression '*' powExpression
    |   multiplicativeExpression '/' powExpression
    ;

expression
    :   multiplicativeExpression
    |   expression '+' multiplicativeExpression
    |   expression '-' multiplicativeExpression
    |   '-' expression
    |   '+' expression
    ;

number
    :   Integer ('.' Integer?)?
    ;

Integer
    :   [0-9]+
    ;

Identifier
    :   [a-zA-Z][a-zA-Z0-9_]*
    ;

Whitespace
    :   [ \t]+  -> skip
    ;

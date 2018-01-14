grammar expr;

unit
    :   Identifier
    |   Number
    |   '(' expression ')'
    |   functionExpression
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
    ;

Number
    :   [0-9]+('.'[0-9]+)?
    ;

Identifier
    :   [a-zA-Z]+[a-zA-Z0-9_]*
    ;

Whitespace
    :   [ \t]+  -> skip
    ;

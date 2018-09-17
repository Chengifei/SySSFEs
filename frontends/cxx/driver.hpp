#include <sstream>
#include "exprLexer.h"
#include "exprVisitor.h"

support::Expr parse(std::string str) {
    using namespace antlr4;
    std::stringstream s(std::move(str));
    ANTLRInputStream input(s);
    exprLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    exprParser parser(&tokens);

    exprParser::ExpressionContext *tree = parser.expression();
    support::Expr expr = visitExpression(tree);
    return expr;
}

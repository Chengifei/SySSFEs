#include <sstream>

#include "antlr4-runtime.h"
#include "exprLexer.h"
#include "exprVisitor.h"

support::Expr parse(const char* str) {
    using namespace antlr4;
    std::stringstream s(str);
    ANTLRInputStream input(s);
    exprLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    exprParser parser(&tokens);

    exprParser::ExpressionContext *tree = parser.expression();
    exprVisitor visitor;
    visitor.visitExpression(tree);
    support::Expr expr = std::move(visitor.expr);
    return expr;
}

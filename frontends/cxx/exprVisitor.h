
// Generated from expr.g4 by ANTLR 4.7.1

#pragma once


#include "antlr4-runtime.h"
#include "exprParser.h"
#include <support/Expr.hpp>



/**
 * This class defines an abstract visitor for a parse tree
 * produced by exprParser.
 */
class  exprVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:
    support::Expr expr;
  /**
   * Visit parse trees produced by exprParser.
   */
    virtual antlrcpp::Any visitUnit(exprParser::UnitContext *ctx) {
        if (ctx->Identifier() || ctx->Number()) {
            std::string* data = new std::string(ctx->children[0]->getText());
            expr = support::Expr();
            expr.data = reinterpret_cast<const char*>(data);
        }
        else if (ctx->functionExpression())
            ctx->children[0]->accept(this);
        else
            ctx->children[1]->accept(this);
        return nullptr;
    }

    virtual antlrcpp::Any visitFunctionExpression(exprParser::FunctionExpressionContext *ctx) {}

    virtual antlrcpp::Any visitPowExpression(exprParser::PowExpressionContext *ctx) {
        if (ctx->children.size() == 3) {
            std::string* data = new std::string(ctx->children[1]->getText());
            support::Expr root(new support::Expr::Op(2, data));
            ctx->children[0]->accept(this);
            root.op->args[0] = std::move(expr);
            ctx->children[2]->accept(this);
            root.op->args[1] = std::move(expr);
            expr = std::move(root);
        }
        else {
            ctx->children[0]->accept(this);
        }
        return nullptr;
    }

    virtual antlrcpp::Any visitMultiplicativeExpression(exprParser::MultiplicativeExpressionContext *ctx) {
        if (ctx->children.size() == 3) {
            std::string* data = new std::string(ctx->children[1]->getText());
            support::Expr root(new support::Expr::Op(2, data));
            ctx->children[0]->accept(this);
            root.op->args[0] = std::move(expr);
            ctx->children[2]->accept(this);
            root.op->args[1] = std::move(expr);
            expr = std::move(root);
        }
        else {
            ctx->children[0]->accept(this);
        }
        return nullptr;
    }

    virtual antlrcpp::Any visitExpression(exprParser::ExpressionContext *ctx) {
        if (ctx->children.size() == 3) {
            std::string* data = new std::string(ctx->children[1]->getText());
            support::Expr root(new support::Expr::Op(2, data));
            ctx->children[0]->accept(this);
            root.op->args[0] = std::move(expr);
            ctx->children[2]->accept(this);
            root.op->args[1] = std::move(expr);
            expr = std::move(root);
        }
        else {
            ctx->children[0]->accept(this);
        }
        return nullptr;
    }
};


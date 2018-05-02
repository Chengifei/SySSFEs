/* Copyright 2017-2018 by Yifei Zheng
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "exprVisitor.hpp"
#include "operator_map.hpp"
#include "operators.hpp"

support::Expr visitUnit(exprParser::UnitContext* ctx);
support::Expr visitFunctionExpression(exprParser::FunctionExpressionContext* ctx);
support::Expr visitPowExpression(exprParser::PowExpressionContext* ctx);
support::Expr visitMultiplicativeExpression(exprParser::MultiplicativeExpressionContext* ctx);

support::Expr visitUnit(exprParser::UnitContext* ctx) {
    if (ctx->Identifier()) {
        std::string* data = new std::string(ctx->children[0]->getText());
        support::Expr expr;
        expr.type = static_cast<long long>(NODE_TYPE::STRING);
        expr.data = reinterpret_cast<char*>(data);
        return expr;
    }
    else if (ctx->Number()) {
        double d = std::strtod(ctx->children[0]->getText().c_str(), nullptr);
        support::Expr expr;
        expr.type = static_cast<long long>(NODE_TYPE::NUMBER);
        expr.data = reinterpret_cast<char*>(new double(d));
        return expr;
    }
    else if (ctx->functionExpression())
        return visitFunctionExpression(static_cast<exprParser::FunctionExpressionContext*>(ctx->children[0]));
    else
        return visitExpression(static_cast<exprParser::ExpressionContext*>(ctx->children[1]));
}

support::Expr visitFunctionExpression(exprParser::FunctionExpressionContext* ctx) {
    // minus 3 for function name, lparen, rparen, plus one for pairing up
    // the last argument with comma
    std::size_t sz = (ctx->children.size() - 3 + 1) / 2;
    support::Expr root(new support::Expr::Op(sz, function_map[ctx->children[0]->getText()]));
    auto it = ctx->children.cbegin() + 2;
    for (std::size_t i = 0; i != sz; ++i, it += 2) {
        root.op->args[i] = visitExpression(static_cast<exprParser::ExpressionContext*>(*it));
    }
    return root;
}

support::Expr visitPowExpression(exprParser::PowExpressionContext* ctx) {
    if (ctx->children.size() == 3) {
        support::Expr root(new support::Expr::Op(2, &POW_OP));
        root.op->args[0] = visitPowExpression(static_cast<exprParser::PowExpressionContext*>(ctx->children[0]));
        root.op->args[1] = visitUnit(static_cast<exprParser::UnitContext*>(ctx->children[2]));
        return root;
    }
    else {
        return visitUnit(static_cast<exprParser::UnitContext*>(ctx->children[0]));
    }
}

support::Expr visitMultiplicativeExpression(exprParser::MultiplicativeExpressionContext* ctx) {
    if (ctx->children.size() == 3) {
        support::Expr root(new support::Expr::Op(2, ctx->children[1]->getText() == "*" ? &MUL_OP : &DIV_OP));
        root.op->args[0] = visitMultiplicativeExpression(static_cast<exprParser::MultiplicativeExpressionContext*>(ctx->children[0]));
        root.op->args[1] = visitPowExpression(static_cast<exprParser::PowExpressionContext*>(ctx->children[2]));
        return root;
    }
    else {
        return visitPowExpression(static_cast<exprParser::PowExpressionContext*>(ctx->children[0]));
    }
}

support::Expr visitExpression(exprParser::ExpressionContext* ctx) {
    if (ctx->children.size() == 3) {
        support::Expr root(new support::Expr::Op(2, ctx->children[1]->getText() == "+" ? &PLUS_OP : &MINUS_OP));
        root.op->args[0] = visitExpression(static_cast<exprParser::ExpressionContext*>(ctx->children[0]));
        root.op->args[1] = visitMultiplicativeExpression(static_cast<exprParser::MultiplicativeExpressionContext*>(ctx->children[2]));
        return root;
    }
    else if (ctx->children.size() == 2) {
        if (ctx->children[0]->getText() == "+")
            return visitExpression(static_cast<exprParser::ExpressionContext*>(ctx->children[1]));
        else {
            support::Expr root(new support::Expr::Op(1, new std::string("neg")));
            root.op->args[0] = visitExpression(static_cast<exprParser::ExpressionContext*>(ctx->children[1]));
            return root;
        }
    }
    else {
        return visitMultiplicativeExpression(static_cast<exprParser::MultiplicativeExpressionContext*>(ctx->children[0]));
    }
}

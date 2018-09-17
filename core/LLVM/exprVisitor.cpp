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

support::Expr expr_reader::visitUnit(exprParser::UnitContext* ctx) {
    if (ctx->Identifier()) {
        std::string* data = new std::string(ctx->children[0]->getText());
        support::Expr expr;
        expr.type = static_cast<long long>(NODE_TYPE::VARIABLE);
        expr.data = reinterpret_cast<char*>(data);
        return expr;
    }
    else if (auto c = dynamic_cast<exprParser::CombIdentContext*>(ctx->children[0])) {
        return visitCombIdent(c);
    }
    else if (auto c = dynamic_cast<exprParser::MemIdentContext*>(ctx->children[0])) {
        return visitMemIdent(c);
    }
    else if (auto c = dynamic_cast<exprParser::NumberContext*>(ctx->children[0])) {
        return visitNumber(c);
    }
    else if (ctx->functionExpression())
        return visitFunctionExpression(static_cast<exprParser::FunctionExpressionContext*>(ctx->children[0]));
    else
        return visitExpression(static_cast<exprParser::ExpressionContext*>(ctx->children[1]));
}

support::Expr expr_reader::visitCombIdent(exprParser::CombIdentContext* ctx) {
    auto data = new cmb_info{std::stoul(ctx->children[1]->getText().c_str()), ctx->Identifier()->getText()};
    support::Expr expr;
    expr.type = static_cast<long long>(NODE_TYPE::COMBFIELD);
    expr.data = reinterpret_cast<char*>(data);
    max_comb = std::max(max_comb, data->idx);
    return expr;
}

support::Expr expr_reader::visitMemIdent(exprParser::MemIdentContext* ctx) {
    std::string* data = new std::string(ctx->children[3]->getText());
    support::Expr expr;
    expr.type = static_cast<long long>(NODE_TYPE::MEMFIELD);
    expr.data = reinterpret_cast<char*>(data);
    return expr;
}

support::Expr expr_reader::visitNumber(exprParser::NumberContext* ctx) {
    support::Expr expr;
    char* _;
    union {
        double d;
        char* p;
    } a;
    a.d = std::strtod(ctx->children[0]->getText().c_str(), &_);
    expr.type = static_cast<long long>(NODE_TYPE::NUMBER);
    expr.data = a.p;
    return expr;
}

support::Expr expr_reader::visitFunctionExpression(exprParser::FunctionExpressionContext* ctx) {
    // minus 3 for function name, lparen, rparen, plus one for pairing up
    // the last argument with comma
    std::size_t sz = (ctx->children.size() - 3 + 1) / 2;
    support::Expr root(new support::Expr::Op(sz, function_map[ctx->children[0]->getText()]));
    if (root.op->op_data == &DIFF_OP)
        ++max_order[0];
    auto it = ctx->children.cbegin() + 2;
    for (std::size_t i = 0; i != sz; ++i, it += 2) {
        root.op->args[i] = visitExpression(static_cast<exprParser::ExpressionContext*>(*it));
    }
    return root;
}

support::Expr expr_reader::visitPowExpression(exprParser::PowExpressionContext* ctx) {
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

support::Expr expr_reader::visitMultiplicativeExpression(exprParser::MultiplicativeExpressionContext* ctx) {
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

support::Expr expr_reader::visitExpression(exprParser::ExpressionContext* ctx) {
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

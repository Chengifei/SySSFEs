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

#ifndef EXPR_VISITOR_HPP
#define EXPR_VISITOR_HPP
#include <exprParser.h>
#include <support/Expr.hpp>
#include <support/variable.hpp>

struct cmb_info {
    std::size_t idx;
    std::string mem_name;
};

struct mmb_info {
    std::string name;
    std::string mem_name;
};

struct expr_reader {
    support::order_t max_order {};
    std::size_t max_comb = 0;
    support::Expr visitExpression(exprParser::ExpressionContext* ctx);
    support::Expr visitUnit(exprParser::UnitContext* ctx);
    support::Expr visitCombIdent(exprParser::CombIdentContext* ctx);
    support::Expr visitMemIdent(exprParser::MemIdentContext* ctx);
    support::Expr visitNumber(exprParser::NumberContext* ctx);
    support::Expr visitFunctionExpression(exprParser::FunctionExpressionContext* ctx);
    support::Expr visitPowExpression(exprParser::PowExpressionContext* ctx);
    support::Expr visitMultiplicativeExpression(exprParser::MultiplicativeExpressionContext* ctx);
};

#endif

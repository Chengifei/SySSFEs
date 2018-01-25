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
#include "exprParser.h"
#include <support/Expr.hpp>
#include "operator_map.hpp"

// We don't return anything, but it's required for antlr4
class exprVisitor {
public:
    support::Expr expr;
    int visitUnit(exprParser::UnitContext* ctx);
    int visitFunctionExpression(exprParser::FunctionExpressionContext* ctx);
    int visitPowExpression(exprParser::PowExpressionContext* ctx);
    int visitMultiplicativeExpression(exprParser::MultiplicativeExpressionContext* ctx);
    int visitExpression(exprParser::ExpressionContext* ctx);
};
#endif

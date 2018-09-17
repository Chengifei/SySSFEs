/* Copyright 2018 by Yifei Zheng
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

#include <sstream>
#include <exprParser.h>
#include <exprLexer.h>
#include "exprVisitor.hpp"

support::Expr parse(std::string str, std::size_t& comb, std::size_t& order) {
    using namespace antlr4;
    std::stringstream s(std::move(str));
    ANTLRInputStream input(s);
    exprLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    exprParser parser(&tokens);

    exprParser::ExpressionContext *tree = parser.expression();
    expr_reader er;
    auto ret = er.visitExpression(tree);
    comb = er.max_comb;
    order = er.max_order[0];
    return ret;
}

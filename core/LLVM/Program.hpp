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

#ifndef PROGRAM_HPP
#define PROGRAM_HPP
#include <vector>
#include <support/Expr.hpp>
#include "initialization.hpp"
#include "composite_decl.hpp"

struct Program {
    composite_decl object_type;
    initializer initer;
    std::vector<support::Expr> rules;
};
#endif

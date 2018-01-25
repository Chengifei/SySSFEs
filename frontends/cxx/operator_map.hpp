/* Copyright 2017-2018 by Yifei Zheng
 *
 * Please avoid bare macro definition, and always use #define ABC (value).
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

#ifndef OPMAP_HPP
#define OPMAP_HPP
enum class NODE_TYPE : long long {
    FUNC = -2,
    OP = support::Expr::OP,
    DATA = 0
};

enum class OPCODE : long long {
    PLUS = 0,
    MINUS,
    MUL,
    DIV,
    POW
};
#endif

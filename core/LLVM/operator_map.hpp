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
    OP = -1,
    VARIABLE = 0,
    MEMFIELD,
    COMBFIELD,
    NUMBER
};

enum ATOM_TYPE : long long {
    ATOM_NODE_CONST = 1,
    ATOM_NODE_TMPVAR,
    ATOM_NODE_FIELD, // named-object field
    ATOM_NODE_CMBFIELD,
    ATOM_NODE_OP = -1,
    ATOM_NODE_COMPILED = -2
};
#endif

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

#ifndef INITIALIZATION_HPP
#define INITIALIZATION_HPP
#include <vector>
#include "variable.hpp"

struct initializer {
    std::vector<CtrlVariable> controls;
    std::vector<Variable> initializations;
    initializer() {
        controls.reserve(4);
    }
    void add_ctrl(const char* name, double start, double step, double end);
    Variable& make_var(support::type tp, const char* name);
};
#endif

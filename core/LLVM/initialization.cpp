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

#include "initialization.hpp"

void* to_ptr(double d) {
    union {
        double f;
        void* l;
    } u;
    u.f = d;
    return u.l;
}

void initializer::add_ctrl(const char* name, double start, double step, double end) {
    initializations.push_back(CtrlVariable{Variable{support::type{support::type::REAL}, name, to_ptr(start)}, to_ptr(step), to_ptr(end)});
}

Variable& initializer::make_var(support::type tp, const char* name) {
    initializations.push_back(Variable{tp, name});
    return initializations.back();
}

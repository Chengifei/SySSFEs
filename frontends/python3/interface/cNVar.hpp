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

#include "../common/python_common.hpp"
#include "rule_types.hpp"

struct cNVar : PyObject, Variable {
    const char* name() const {
        return static_cast<const Variable*>(this)->name().c_str();
    }
    void need_update(bool flag) {
        _need_update = flag;
    }
    void can_start(bool flag) {
        _can_start = flag;
    }
};

int cNVar_init(PyObject* self, PyObject* args, PyObject*);
extern PyTypeObject cNVarType;

template <class... T>
PyObject* make_NVar(T&&... args) {
    cNVar* inst = static_cast<cNVar*>(cNVarType.tp_alloc(&cNVarType, 0));
    new(static_cast<Variable*>(inst)) Variable(std::forward<T>(args)...);
    return inst;
}


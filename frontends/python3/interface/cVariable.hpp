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

#ifndef ATOM_CODEGEN_SUPPORT_CVARIABLE_HPP
#define ATOM_CODEGEN_SUPPORT_CVARIABLE_HPP
#include "../../common/python_common.hpp"
#include "Variable.hpp"
struct cVariable : PyObject {
    Variable var;
    PyObject* val;
};

PyObject* cVariable_getattr(PyObject* self, char* attr_name);
int cVariable_setattr(PyObject* self, char* attr_name, PyObject* val);
PyObject* cVariable_repr(PyObject* self);
int cVariable_init(PyObject* self, PyObject* args, PyObject*);

extern PyTypeObject cVariableType;
#endif

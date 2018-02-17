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

#ifndef ATOMS_CODEGEN_SUPPORT_CTYPES_HPP
#define ATOMS_CODEGEN_SUPPORT_CTYPES_HPP
#include <python_common.hpp>
#include <support/type.hpp>
struct cTypes : PyObject {
    support::type type;
};

typedef PyObject BaseEnum;

PyObject* cTypes_cmp(PyObject* self, PyObject* rhs, int op);
PyObject* cTypes_getattro(PyObject* self, PyObject* attr_name);
PyObject* cTypes_repr(PyObject* self);
Py_hash_t cTypes_hash(PyObject* self);
PyObject* cTypes_to_C_type(PyObject* self, PyObject*);
int cTypes_init(PyObject* self, PyObject* args, PyObject*);

extern PyTypeObject cTypesType;
extern PyTypeObject cTypesBaseEnum;
#endif

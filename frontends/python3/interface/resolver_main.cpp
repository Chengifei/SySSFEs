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
 *
 * This file exports the full resolver module.
 */

#include "../common/python_common.hpp"
#include "cNVar.hpp"
#include "cRule.hpp"

PyObject* resolve(PyObject*, PyObject* args);

static PyMethodDef ResolverMethods[] = {
    {"resolve", resolve, METH_VARARGS, ""},
    {nullptr}
};

static PyModuleDef ResolverModule = {
    PyModuleDef_HEAD_INIT,
    "resolver",
    "",
    -1,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC
PyInit_resolver() {
    PyObject* m = PyExc(PyModule_Create(&ResolverModule), nullptr);
    PyOnly(PyType_Ready(&cNVarType), 0);
    Py_INCREF(&cNVarType);
    PyModule_AddObject(m, "cNVar", reinterpret_cast<PyObject*>(&cNVarType));
    PyOnly(PyType_Ready(&cRuleType), 0);
    Py_INCREF(&cRuleType);
    PyModule_AddObject(m, "cRule", reinterpret_cast<PyObject*>(&cRuleType));
    PyModule_AddFunctions(m, ResolverMethods);
    return m;
}

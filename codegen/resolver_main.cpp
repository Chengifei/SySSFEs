/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
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

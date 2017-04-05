#include "Python.h"
#include "python_interface.hpp"

static PyMethodDef ResolverMethods[] = {
    {"resolve", resolve, METH_O, ""},
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
    PyModule_AddFunctions(m, ResolverMethods);
    return m;
}

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
 * This file exports the llvm module.
 */

#include <python_common.hpp>
#include "cTypes.hpp"
#include "Module.hpp"

PyObject* write(PyObject*, PyObject* args) {
    PyObject* program;
    PyObject* options;
    const char* fn;
    if (!PyArg_ParseTuple(args, "OOy", &program, &options, &fn))
        return nullptr;
    if (options != Py_None) {
        PyErr_Format(PyExc_ValueError, "options must be None for now.");
        return nullptr;
    }
    Program& prog = *static_cast<cModule*>(program);
    Module mod;
    Builder builder(prog, mod);
    if (auto ret = builder.compile(); ret == -1)
        return PyErr_Format(PyExc_Exception, "Backend failed to resolve input to canonical updating order", ret);
    else if (ret)
        return PyErr_Format(PyExc_Exception, "Backend failed to verify compiled module");
    return PyLong_FromLong(mod.write(fn));
}

static PyModuleDef llvmModule = {
    PyModuleDef_HEAD_INIT,
    "llvm",
    "",
    -1,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

static PyMethodDef llvmMethods[] = {
    {"write", &write, METH_VARARGS, nullptr},
    {nullptr}
};

PyMODINIT_FUNC
PyInit_llvm() {
    PyObject* m = PyExc(PyModule_Create(&llvmModule), nullptr);
    PyOnly(PyType_Ready(&cModule_type), 0);
    Py_INCREF(&cModule_type);
    PyModule_AddObject(m, "Module", reinterpret_cast<PyObject*>(&cModule_type));
    PyOnly(PyType_Ready(&cTypesType), 0);
    Py_INCREF(&cTypesType);
    PyModule_AddObject(m, "cTypes", reinterpret_cast<PyObject*>(&cTypesType));
    PyOnly(PyType_Ready(&cTypesBaseEnum), 0);
    PyDict_SetItemString(cTypesBaseEnum.tp_dict, "REAL", PyLong_FromLong(support::type::REAL));
    PyDict_SetItemString(cTypesBaseEnum.tp_dict, "INT", PyLong_FromLong(support::type::INT));
    PyDict_SetItemString(cTypesBaseEnum.tp_dict, "BUFFER", PyLong_FromLong(support::type::BUFFER));
    PyDict_SetItemString(cTypesType.tp_dict, "BaseEnum", reinterpret_cast<PyObject*>(&cTypesBaseEnum));
    Py_INCREF(&cTypesBaseEnum);
    PyModule_AddFunctions(m, llvmMethods);
    return m;
}

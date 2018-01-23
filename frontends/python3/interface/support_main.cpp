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
 * This file exports the csrc_tracker module.
 */

#include "../../common/python_common.hpp"
#include "cSrcLocation.hpp"
#include "cTypes.hpp"
#include "cVariable.hpp"
#include "cExpr.hpp"

static PyModuleDef SupportModule = {
        PyModuleDef_HEAD_INIT,
        "support",
        "",
        -1,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
};

PyMODINIT_FUNC
PyInit_support() {
    PyObject* m = PyExc(PyModule_Create(&SupportModule), nullptr);
    PyOnly(PyType_Ready(&csrc_trackerType), 0);
    Py_INCREF(&csrc_trackerType);
    PyModule_AddObject(m, "cSrcLocation", reinterpret_cast<PyObject*>(&csrc_trackerType));
    PyOnly(PyType_Ready(&cTypesType), 0);
    PyOnly(PyType_Ready(&cTypesBaseEnum), 0);
    PyDict_SetItemString(cTypesBaseEnum.tp_dict, "DOUBLE", PyLong_FromLong(Types::DOUBLE));
    PyDict_SetItemString(cTypesBaseEnum.tp_dict, "INT", PyLong_FromLong(Types::INT));
    PyDict_SetItemString(cTypesBaseEnum.tp_dict, "BUFFER", PyLong_FromLong(Types::BUFFER));
    PyDict_SetItemString(cTypesType.tp_dict, "BaseEnum", reinterpret_cast<PyObject*>(&cTypesBaseEnum));
    Py_INCREF(&cTypesBaseEnum);
    Py_INCREF(&cTypesType);
    PyModule_AddObject(m, "cTypes", reinterpret_cast<PyObject*>(&cTypesType));
    PyOnly(PyType_Ready(&cVariableType), 0);
    Py_INCREF(&cVariableType);
    PyOnly(PyType_Ready(&cExprType), 0);
    PyDict_SetItemString(cExprType.tp_dict, "PLUS", PyLong_FromLong(OP::PLUS));
    PyDict_SetItemString(cExprType.tp_dict, "MINUS", PyLong_FromLong(OP::MINUS));
    PyDict_SetItemString(cExprType.tp_dict, "MUL", PyLong_FromLong(OP::MUL));
    PyDict_SetItemString(cExprType.tp_dict, "DIV", PyLong_FromLong(OP::DIV));
    PyDict_SetItemString(cExprType.tp_dict, "POW", PyLong_FromLong(OP::POW));
    Py_INCREF(&cExprType);
    PyModule_AddObject(m, "cExpr", reinterpret_cast<PyObject*>(&cExprType));
    PyModule_AddObject(m, "cVariable", reinterpret_cast<PyObject*>(&cVariableType));
    return m;
}

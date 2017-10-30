/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This file exports the csrc_tracker module.
 */
#include "../../common/python_common.hpp"
#include "cSrcLocation.hpp"
#include "cTypes.hpp"
#include "cVariable.hpp"

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
    PyModule_AddObject(m, "cVariable", reinterpret_cast<PyObject*>(&cVariableType));
    return m;
}

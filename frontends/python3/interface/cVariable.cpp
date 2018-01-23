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

#include <new>
#include "cVariable.hpp"
#include "cTypes.hpp"

PyObject* cVariable_getattr(PyObject* self, char* attr_name) {
    if (strcmp(attr_name, "type") == 0) {
        // This relies on cTypes being a immutable Python type.
        cTypes* obj = static_cast<cTypes*>(cTypesType.tp_new(&cTypesType, nullptr, nullptr));
        new(&obj->type) support::type(static_cast<cVariable*>(self)->var.t);
        return obj;
    }
    else if (PyObject* ret = PyDict_GetItemString(static_cast<cVariable*>(self)->dict, attr_name))
        return inc_prpg(ret);
    PyErr_Format(PyExc_AttributeError,
                 "cVariable object has no attribute '%.400s'", attr_name);
    return nullptr;
}

int cVariable_setattr(PyObject* self, char* attr_name, PyObject* val) {
    if (strcmp(attr_name, "type") == 0) {
        PyErr_Format(PyExc_AttributeError,
                     "Attribute 'type' is readonly", attr_name);
        return -1;
    }
    if (val)
        PyDict_SetItemString(static_cast<cVariable*>(self)->dict, attr_name, val);
    return 0;
}

PyObject* cVariable_repr(PyObject* self) {
    cVariable& var = *static_cast<cVariable*>(self);
    return PyUnicode_FromFormat("%U", cTypes_repr(cVariable_getattr(self, "type")));
}

int cVariable_init(PyObject* self, PyObject* args, PyObject*) {
    PyObject* type;
    PyObject* val = Py_None;
    if (!PyArg_ParseTuple(args, "O|O", &type, &val))
        return -1;
    new(&(static_cast<cVariable*>(self)->var)) support::variable{static_cast<cTypes*>(type)->type};
    Py_INCREF(val);
    static_cast<cVariable*>(self)->dict = PyDict_New();
    cVariable_setattr(self, "val", val);
    return 0;
}

PyTypeObject cVariableType {
        PyVarObject_HEAD_INIT(NULL, 0)
        "cVariable.SrcLocation", /* tp_name */
        sizeof(cVariable),      /* tp_basicsize */
        0,                         /* tp_itemsize */
        call_destructor<cVariable>, /* tp_dealloc */
        0,                         /* tp_print */
        cVariable_getattr,             /* tp_getattr */
        cVariable_setattr,                /* tp_setattr */
        0,                         /* tp_reserved */
        cVariable_repr,                /* tp_repr */
        0,                         /* tp_as_number */
        0,                         /* tp_as_sequence */
        0,                         /* tp_as_mapping */
        0,                         /* tp_hash  */
        0,                         /* tp_call */
        0,                         /* tp_str */
        0,                         /* tp_getattro */
        0,                         /* tp_setattro */
        0,                         /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,        /* tp_flags */
        0,                         /* tp_doc */
        0,                         /* tp_traverse */
        0,                         /* tp_clear */
        0,                         /* tp_richcompare */
        0,                         /* tp_weaklistoffset */
        0,                         /* tp_iter */
        0,                         /* tp_iternext */
        0,                         /* tp_methods */
        0,                         /* tp_members */
        0,                         /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        cVariable_init,                /* tp_init */
        0,                         /* tp_alloc */
        PyType_GenericNew          /* tp_new */
};
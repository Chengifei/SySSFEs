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
#include "cTypes.hpp"
#include <structmember.h>

PyObject* cTypes_cmp(PyObject* self, PyObject* rhs, int op) {
    switch (op) {
        case Py_EQ:
            return PyBool_FromLong(static_cast<cTypes*>(self)->type.base == static_cast<cTypes*>(rhs)->type.base
                                   && static_cast<cTypes*>(self)->type.is_const == static_cast<cTypes*>(rhs)->type.is_const
                                   && static_cast<cTypes*>(self)->type.agg == static_cast<cTypes*>(rhs)->type.agg);
    }
    return Py_NotImplemented;
}

PyObject* cTypes_getattro(PyObject* self, PyObject* attr_name) {
    if (PyObject* meth = _PyType_Lookup(&cTypesType, attr_name)) {
        Py_INCREF(meth);
        descrgetfunc f = meth->ob_type->tp_descr_get;
        Py_DECREF(meth);
        return f(meth, self, reinterpret_cast<PyObject*>(&cTypesType));
    }
    else if (PyUnicode_CompareWithASCIIString(attr_name, "base") == 0)
        switch (static_cast<cTypes*>(self)->type.base) {
            case support::type::REAL:
                return PyUnicode_FromString("double");
            case support::type::INT:
                return PyUnicode_FromString("int");
            case support::type::BUFFER:
                return PyUnicode_FromString("buf");
        }
    else if (PyUnicode_CompareWithASCIIString(attr_name, "qual") == 0)
        return PyLong_FromLong(static_cast<cTypes*>(self)->type.is_const);
    else if (PyUnicode_CompareWithASCIIString(attr_name, "agg") == 0)
        return PyLong_FromLong(static_cast<cTypes*>(self)->type.agg);
    PyErr_Format(PyExc_AttributeError,
                 "cTypes object has no attribute '%.400s'", attr_name);
    return nullptr;
}

PyObject* cTypes_repr(PyObject* self) {
    cTypes& type = *static_cast<cTypes*>(self);
    char buf[32] {};
    switch (type.type.base) {
        case support::type::REAL:
            strcat(buf, "double");
            break;
        case support::type::INT:
            strcat(buf, "int");
            break;
        case support::type::BUFFER:
            strcat(buf, "buf");
            break;
    }
    if (type.type.is_const) {
        strcat(buf, " const");
    }
    if (type.type.agg)
        return PyUnicode_FromFormat("%s [%zu]", buf, type.type.agg);
    return PyUnicode_FromString(buf);
}

Py_hash_t cTypes_hash(PyObject* self) {
    cTypes& type = *static_cast<cTypes*>(self);
    return type.type.base << sizeof(char)*2 | type.type.is_const << sizeof(char) | type.type.agg ;
}

int cTypes_init(PyObject* self, PyObject* args, PyObject*) {
    decltype(support::type::REAL) base;
    unsigned is_const;
    unsigned agg;
    if (!PyArg_ParseTuple(args, "III", &base, &is_const, &agg))
        return -1;
    new(&(static_cast<cTypes*>(self)->type)) support::type{base, static_cast<bool>(is_const), -1, agg};
    return 0;
}

PyObject* cTypes_to_C_type(PyObject* self, PyObject*) {
    cTypes& type = *static_cast<cTypes*>(self);
    char tp[22] {};
    if (type.type.is_const)
        memcpy(tp, "const ", 6); // FIXME: Maintainence hell, strlen must agree
    switch (type.type.base) {
        case support::type::REAL:
            strcat(tp, "double");
            break;
        case support::type::INT:
            strcat(tp, "int");
            break;
        case support::type::BUFFER:
            strcat(tp, "char");
            break;
    }
    if (type.type.agg) {
        // FIXME: Dynamic buffer
        if (type.type.agg > 65536) // MAGIC NUMBER, ensure buffer size is enough
            return nullptr; // But anyway this shouldn't fail
        sprintf(tp + strlen(tp), " %%s[%i]", type.type.agg);
    }
    else {
        sprintf(tp + strlen(tp), " %%s");
    }
    return PyUnicode_FromString(tp);
}

PyMethodDef Methods[] {
        {"get_C_type", cTypes_to_C_type, METH_NOARGS, nullptr},
        {nullptr}
};

PyTypeObject cTypesType {
        PyVarObject_HEAD_INIT(NULL, 0)
        "support.cTypes", /* tp_name */
        sizeof(cTypes),      /* tp_basicsize */
        0,                         /* tp_itemsize */
        call_destructor<cTypes>, /* tp_dealloc */
        0,                         /* tp_print */
        0,                         /* tp_getattr */
        0,                         /* tp_setattr */
        0,                         /* tp_reserved */
        cTypes_repr,               /* tp_repr */
        0,                         /* tp_as_number */
        0,                         /* tp_as_sequence */
        0,                         /* tp_as_mapping */
        cTypes_hash,               /* tp_hash  */
        0,                         /* tp_call */
        0,                         /* tp_str */
        cTypes_getattro,           /* tp_getattro */
        0,                         /* tp_setattro */
        0,                         /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,        /* tp_flags */
        0,                         /* tp_doc */
        0,                         /* tp_traverse */
        0,                         /* tp_clear */
        cTypes_cmp,                /* tp_richcompare */
        0,                         /* tp_weaklistoffset */
        0,                         /* tp_iter */
        0,                         /* tp_iternext */
        Methods,                   /* tp_methods */
        0,                         /* tp_members */
        0,                         /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        cTypes_init,                /* tp_init */
        0,                         /* tp_alloc */
        PyType_GenericNew          /* tp_new */
};

PyTypeObject cTypesBaseEnum {
        PyVarObject_HEAD_INIT(NULL, 0)
        "support.cTypes.BaseEnum", /* tp_name */
        sizeof(BaseEnum),          /* tp_basicsize */
};

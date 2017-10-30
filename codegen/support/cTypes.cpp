//
// Created by Yifei Zheng on 08/09/2017.
//

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

PyObject* cTypes_getattr(PyObject* self, char* attr_name) {
    if (strcmp(attr_name, "base") == 0)
        switch (static_cast<cTypes*>(self)->type.base) {
            case Types::DOUBLE:
                return PyUnicode_FromString("double");
            case Types::INT:
                return PyUnicode_FromString("int");
            case Types::BUFFER:
                return PyUnicode_FromString("buf");
        }
    else if (strcmp(attr_name, "qual") == 0)
        return PyLong_FromLong(static_cast<cTypes*>(self)->type.is_const);
    else if (strcmp(attr_name, "agg") == 0)
        return PyLong_FromLong(static_cast<cTypes*>(self)->type.agg);
    PyErr_Format(PyExc_AttributeError,
                 "cTypes object has no attribute '%.400s'", attr_name);
    return nullptr;
}

PyObject* cTypes_repr(PyObject* self) {
    cTypes& type = *static_cast<cTypes*>(self);
    char buf[32] {};
    switch (type.type.base) {
        case Types::DOUBLE:
            strcat(buf, "double");
            break;
        case Types::INT:
            strcat(buf, "int");
            break;
        case Types::BUFFER:
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
    decltype(Types::DOUBLE) base;
    unsigned is_const;
    int agg;
    if (!PyArg_ParseTuple(args, "IIi", &base, &is_const, &agg))
        return -1;
    new(&(static_cast<cTypes*>(self)->type)) Types{base, static_cast<bool>(is_const), agg};
    return 0;
}

PyTypeObject cTypesType {
        PyVarObject_HEAD_INIT(NULL, 0)
        "support.cTypes", /* tp_name */
        sizeof(cTypes),      /* tp_basicsize */
        0,                         /* tp_itemsize */
        call_destructor<cTypes>, /* tp_dealloc */
        0,                         /* tp_print */
        cTypes_getattr,             /* tp_getattr */
        0,                         /* tp_setattr */
        0,                         /* tp_reserved */
        cTypes_repr,                /* tp_repr */
        0,                         /* tp_as_number */
        0,                         /* tp_as_sequence */
        0,                         /* tp_as_mapping */
        cTypes_hash,                /* tp_hash  */
        0,                         /* tp_call */
        0,                         /* tp_str */
        0,                         /* tp_getattro */
        0,                         /* tp_setattro */
        0,                         /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,        /* tp_flags */
        0,                         /* tp_doc */
        0,                         /* tp_traverse */
        0,                         /* tp_clear */
        cTypes_cmp,                 /* tp_richcompare */
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
        cTypes_init,                /* tp_init */
        0,                         /* tp_alloc */
        PyType_GenericNew          /* tp_new */
};

PyTypeObject cTypesBaseEnum {
        PyVarObject_HEAD_INIT(NULL, 0)
        "support.cTypes.BaseEnum", /* tp_name */
        sizeof(BaseEnum),          /* tp_basicsize */
        .tp_flags = Py_TPFLAGS_DEFAULT,        /* tp_flags */
};

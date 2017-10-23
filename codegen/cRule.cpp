/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This file defines a few data types to represent rules.
 */

#include "cRule.hpp"
#include "support/cSrcLocation.hpp"

PyObject* cRule_getattr(PyObject* self, char* attr_name) {
    if (strcmp(attr_name, "dom") == 0)
        return static_cast<cRule*>(self)->dom;
    else if (strcmp(attr_name, "src") == 0)
        return make_csrc_tracker(static_cast<cRule*>(self)->src);
    else if (strcmp(attr_name, "lhs") == 0)
        return static_cast<cRule*>(self)->lhs;
    else if (strcmp(attr_name, "rhs") == 0)
        return static_cast<cRule*>(self)->rhs;
    PyErr_Format(PyExc_AttributeError,
                 "cRule object has no attribute '%.400s'", attr_name);
    return nullptr;
}

PyObject* cRule_repr(PyObject* self) {
    cRule& var = *static_cast<cRule*>(self);
    return PyUnicode_FromFormat("Rule at %U %R = %R", csrc_tracker_repr(make_csrc_tracker(
            var.src)), var.lhs, var.rhs);
}

int cRule_init(PyObject* self, PyObject* args, PyObject*) {
    PyObject* src, *dom, *lhs, *rhs;
    if (!PyArg_ParseTuple(args, "O!OOO", &csrc_trackerType, &src, &dom, &lhs, &rhs))
        return -1;
    cRule& cself = *static_cast<cRule*>(self);
    Py_INCREF(src);
    cself.src = static_cast<csrc_tracker*>(src)->st;
    Py_INCREF(dom);
    cself.dom = dom;
    Py_INCREF(lhs);
    cself.lhs = lhs;
    Py_INCREF(rhs);
    cself.rhs = rhs;
    return 0;
}

PyTypeObject cRuleType {
        PyVarObject_HEAD_INIT(NULL, 0)
        "resolver.cRule",          /* tp_name */
        sizeof(cRule),             /* tp_basicsize */
        0,                         /* tp_itemsize */
        call_destructor<cRule>,    /* tp_dealloc */
        0,                         /* tp_print */
        cRule_getattr,             /* tp_getattr */
        0,                         /* tp_setattr */
        0,                         /* tp_reserved */
        cRule_repr,                /* tp_repr */
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
        cRule_init,                /* tp_init */
        0,                         /* tp_alloc */
        PyType_GenericNew          /* tp_new */
};

/* Copyright 2017 by Yifei Zheng
* This file is part of ATOM.
* Unauthorized copy, modification or distribution is prohibited.
*
* Implements Python interface for cExpr
*/
#include "cExpr.hpp"

PyObject* cExpr_getattro(PyObject* self, PyObject* attr_name) {
    if (PyObject* descr = PyDict_GetItem(cExprType.tp_dict, attr_name)){
        descrgetfunc f = descr->ob_type->tp_descr_get;
        return f(descr, self, reinterpret_cast<PyObject*>(&cExprType));
    }
    if (!PyUnicode_Check(attr_name)) {
        PyErr_Format(PyExc_TypeError, "Expecting a unicode object, got a %S", attr_name->ob_type);
        return nullptr;
    }
    if (!PyUnicode_CompareWithASCIIString(attr_name, "op"))
        return PyUnicode_FromFormat("<0x%x referring to %x>",
                                    &static_cast<cExpr*>(self)->tree->op,
                                    static_cast<cExpr*>(self)->tree->op->op_data);
    else if (!PyUnicode_CompareWithASCIIString(attr_name, "data"))
        return PyBytes_FromString(static_cast<cExpr*>(self)->tree->data);
    else if (!PyUnicode_CompareWithASCIIString(attr_name, "type"))
        return PyLong_FromLongLong(static_cast<cExpr*>(self)->tree->type);
    else {
        PyErr_Format(PyExc_AttributeError, "Unknown attribute %U", attr_name);
        return nullptr;
    }
}

int cExpr_setattro(PyObject* self, PyObject* attr_name, PyObject* val) {
    if (!PyUnicode_Check(attr_name))
        return -1;
    if (PyUnicode_CompareWithASCIIString(attr_name, "op"))
        // TODO: Perform op conversion here return static_cast<cExpr*>(self)->tree->op;
        return 0;
    else if (PyUnicode_CompareWithASCIIString(attr_name, "data")) {
        const char* data = PyBytes_AsString(val);
        PyExc(data, nullptr);
        static_cast<cExpr*>(self)->tree->data = PyBytes_AsString(val);
        return 0;
    }
    else if (PyUnicode_CompareWithASCIIString(attr_name, "type")) {
        static_cast<cExpr*>(self)->tree->type = PyLong_AsLongLong(val);
        return 0;
    }
    return -1;
}

PyObject* cExpr_repr(PyObject* self) {
    Expr& s = *static_cast<cExpr*>(self)->tree;
    if (s.type == Expr::OP)
        return PyUnicode_FromFormat("<cExpr containing Op(%i)>", s.op->op_data);
    return PyUnicode_FromFormat("<cExpr containing data(%s)>", s.data);
}

int cExpr_init(PyObject* self, PyObject* args, PyObject*) {
    cExpr *s = static_cast<cExpr*>(self);
    PyObject* arg;
    int force_arg_treat_as_argc = 0;
    if (!PyArg_ParseTuple(args, "O|i", &arg, &force_arg_treat_as_argc))
        return -1;
    s->tree = new Expr;
    s->view = false;
    if (PyLong_CheckExact(arg)) {
        s->tree->type = Expr::OP;
        switch (int op = PyLong_AsLong(arg)) {
            default:
                s->tree->op = new Expr::Op(2, reinterpret_cast<void*>(op));
        }
        return 0;
    }
    else if (PyUnicode_CheckExact(arg)) {
        if (force_arg_treat_as_argc) {
            s->tree->type = Expr::OP;
            // see below
            s->tree->op = new Expr::Op(force_arg_treat_as_argc, PyUnicode_AsUTF8(arg));
        }
        else {
            s->tree->type = Expr::DATA;
            // NO Py_INCREF(arg) because Expr doesn't delete it
            // But it's unjustified yet
            s->tree->data = PyUnicode_AsUTF8(arg);
        }
        return 0;
    }
    PyErr_SetString(PyExc_TypeError, "arg must be either int or str");
    return -1;
}

PyObject* make_view(Expr* p) {
    PyObject* view = cExprType.tp_alloc(&cExprType, 0);
    static_cast<cExpr*>(view)->view = true;
    static_cast<cExpr*>(view)->tree = p;
    return view;
}

PyObject* rkid(PyObject* self, Py_ssize_t num) {
    if (static_cast<cExpr*>(self)->tree->type == Expr::OP) {
        if (num < static_cast<cExpr*>(self)->tree->op->argc) {
            return make_view(&static_cast<cExpr*>(self)->tree->op->args[num]);
        }
        else {
            PyErr_Format(PyExc_IndexError, "Max %i",
                         static_cast<cExpr *>(self)->tree->op->argc);
            return nullptr;
        }
    }
    PyErr_SetString(PyExc_ValueError, "No child found");
    return nullptr;
}

int wkid(PyObject* self, Py_ssize_t num, PyObject* val) {
    if (static_cast<cExpr*>(self)->tree->type == Expr::OP) {
        if (num < static_cast<cExpr*>(self)->tree->op->argc) {
            if (static_cast<cExpr*>(val)->view)
                return -1;
            static_cast<cExpr*>(self)->tree->op->args[num] = std::move(*static_cast<cExpr*>(val)->tree);
            return 0;
        }
        else {
            PyErr_Format(PyExc_IndexError, "Max %i", static_cast<cExpr*>(self)->tree->op->argc);
            return -1;
        }
    }
    PyErr_SetString(PyExc_ValueError, "No child found");
    return -1;
}

Py_ssize_t len(PyObject* self) {
    if (static_cast<cExpr*>(self)->tree->type == Expr::OP)
        return static_cast<cExpr*>(self)->tree->op->argc;
    PyErr_SetString(PyExc_ValueError, "Length N/A on non-op nodes");
    return -1;
}

PySequenceMethods methods = {
    len,
    nullptr,
    nullptr,
    rkid,
    nullptr,
    wkid,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyTypeObject cExprType {
        PyVarObject_HEAD_INIT(NULL, 0)
        "support.cExpr",           /* tp_name */
        sizeof(cExpr),             /* tp_basicsize */
        0,                         /* tp_itemsize */
        call_destructor<cExpr>,    /* tp_dealloc */
        0,                         /* tp_print */
        0,                         /* tp_getattr */
        0,                         /* tp_setattr */
        0,                         /* tp_reserved */
        cExpr_repr,                /* tp_repr */
        0,                         /* tp_as_number */
        &methods,                  /* tp_as_sequence */
        0,                         /* tp_as_mapping */
        0,                         /* tp_hash  */
        0,                         /* tp_call */
        0,                         /* tp_str */
        cExpr_getattro,            /* tp_getattro */
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
        cExpr_init,                /* tp_init */
        0,                         /* tp_alloc */
        PyType_GenericNew          /* tp_new */
};

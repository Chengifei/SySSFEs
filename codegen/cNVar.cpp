#include "cNVar.hpp"

PyObject* cNVar_cmp(PyObject* self, PyObject* rhs, int op) {
    switch (op) {
        case Py_EQ:
            return PyBool_FromLong(static_cast<cNVar*>(self)->var == static_cast<cNVar*>(rhs)->var);
    }
    return Py_NotImplemented;
}

PyObject* cNVar_getattr(PyObject* self, char* attr_name) {
    if (strcmp(attr_name, "name") == 0)
        return PyUnicode_FromString(static_cast<cNVar*>(self)->var.name.c_str());
    else if (strcmp(attr_name, "order") == 0) {
        PyObject* L = PyList_New(8);
        for (std::size_t i = 0; i != 8; ++i)
            PyList_SET_ITEM(L, i, PyLong_FromLong(static_cast<cNVar*>(self)->var.order[i]));
        return L;
    }
    PyErr_Format(PyExc_AttributeError,
                 "cNVar object has no attribute '%.400s'", attr_name);
    return nullptr;
}

PyObject* cNVar_repr(PyObject* self) {
    cNVar& var = *static_cast<cNVar*>(self);
    return PyUnicode_FromFormat("%s(%zu)", var.var.name.c_str(), var.var.order);
}

Py_hash_t cNVar_hash(PyObject* self) {
    cNVar& var = *static_cast<cNVar*>(self);
    return hash_value(var.var);
}

int cNVar_init(PyObject* self, PyObject* args, PyObject*) {
    const char* name;
    unsigned order;
    if (!PyArg_ParseTuple(args, "sI", &name, &order))
        return -1;
    new(&(static_cast<cNVar*>(self)->var)) NVar(name, order);
    return 0;
}

PyTypeObject cNVarType {
    PyVarObject_HEAD_INIT(NULL, 0)
    "resolver.cNVar",          /* tp_name */
    sizeof(cNVar),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    call_destructor<cNVar>,    /* tp_dealloc */
    0,                         /* tp_print */
    cNVar_getattr,             /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    cNVar_repr,                /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    cNVar_hash,                /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    0,                         /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    cNVar_cmp,                 /* tp_richcompare */
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
    cNVar_init,                /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew          /* tp_new */
};

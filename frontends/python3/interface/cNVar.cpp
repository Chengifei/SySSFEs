#include "cNVar.hpp"

inline Variable& get_var(PyObject* o) {
    return *static_cast<Variable*>(static_cast<cNVar*>(o));
}

PyObject* cNVar_cmp(PyObject* self, PyObject* rhs, int op) {
    switch (op) {
        case Py_EQ:
            if (rhs->ob_type == &cNVarType)
                return PyBool_FromLong(get_var(self) == get_var(self));
            else if (get_var(self).order()[0] == 0 || PyUnicode_Check(rhs)) { // FIXME: Only order[0]
                if (PyUnicode_READY(rhs)) // 0 for success
                    return nullptr;
                if (!PyUnicode_IS_ASCII(rhs))
                    Py_RETURN_FALSE;
                return PyBool_FromLong(
                        get_var(self).name() == static_cast<char*>(PyUnicode_DATA(rhs))
                );
            }
    }
    return Py_NotImplemented;
}

PyObject* cNVar_getattro(PyObject* self, PyObject* attro) {
    const char* attr_name = PyExc(PyUnicode_AsUTF8(attro), nullptr);
    if (strcmp(attr_name, "name") == 0)
        return PyUnicode_FromString(static_cast<cNVar*>(self)->name());
    else if (strcmp(attr_name, "order") == 0) {
        PyObject* L = PyList_New(8);
        for (std::size_t i = 0; i != 8; ++i)
            PyList_SET_ITEM(L, i, PyLong_FromLong(static_cast<cNVar*>(self)->order()[i]));
        return L;
    }
    PyErr_Format(PyExc_AttributeError,
                 "cNVar object has no attribute '%.400s'", attr_name);
    return nullptr;
}

int cNVar_setattro(PyObject* self, PyObject* attro, PyObject* val) {
    const char* attr_name = PyExc(PyUnicode_AsUTF8(attro), nullptr);
    if (strcmp(attr_name, "need_update") == 0) {
        static_cast<cNVar*>(self)->need_update(PyObject_IsTrue(val));
        return 0;
    }
    else if (strcmp(attr_name, "can_start") == 0) {
        static_cast<cNVar*>(self)->can_start(PyObject_IsTrue(val));
        return 0;
    }
    return -1;
}

PyObject* cNVar_repr(PyObject* self) {
    return PyUnicode_FromFormat("%s(%zu)", 
            static_cast<cNVar*>(self)->name(), static_cast<cNVar*>(self)->order());
}

Py_hash_t cNVar_hash(PyObject* self) {
    if (get_var(self).order()[0] == 0) // FIXME: Only order[0]
        return PyUnicode_Type.tp_hash(PyUnicode_FromString(get_var(self).name().c_str()));
    return hash_value(get_var(self));
}

int cNVar_init(PyObject* self, PyObject* args, PyObject*) {
    const char* name;
    unsigned order;
    if (!PyArg_ParseTuple(args, "sI", &name, &order))
        return -1;
    new(&get_var(self)) Variable(name, order);
    return 0;
}

PyTypeObject cNVarType {
    PyVarObject_HEAD_INIT(NULL, 0)
    "resolver.cNVar",          /* tp_name */
    sizeof(cNVar),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    call_destructor<cNVar>,    /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    cNVar_repr,                /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    cNVar_hash,                /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    cNVar_getattro,            /* tp_getattro */
    cNVar_setattro,            /* tp_setattro */
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

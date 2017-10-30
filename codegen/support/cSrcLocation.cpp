#include <new>
#include "cSrcLocation.hpp"

PyObject* csrc_tracker_cmp(PyObject* self, PyObject* rhs, int op) {
    switch (op) {
        case Py_EQ:
            return PyBool_FromLong(static_cast<csrc_tracker*>(self)->st.lineno == static_cast<csrc_tracker*>(rhs)->st.lineno
                                  && static_cast<csrc_tracker*>(self)->st.colno == static_cast<csrc_tracker*>(rhs)->st.colno);
    }
    return Py_NotImplemented;
}

PyObject* csrc_tracker_getattr(PyObject* self, char* attr_name) {
    if (strcmp(attr_name, "line") == 0)
        return PyLong_FromLong(static_cast<csrc_tracker*>(self)->st.lineno);
    else if (strcmp(attr_name, "col") == 0)
        return PyLong_FromLong(static_cast<csrc_tracker*>(self)->st.colno);
    PyErr_Format(PyExc_AttributeError,
                 "csrc_tracker object has no attribute '%.400s'", attr_name);
    return nullptr;
}

PyObject* csrc_tracker_repr(PyObject* self) {
    csrc_tracker& st = *static_cast<csrc_tracker*>(self);
    return PyUnicode_FromFormat("%zu:%zu", st.st.lineno, st.st.colno);
}

Py_hash_t csrc_tracker_hash(PyObject* self) {
    csrc_tracker& st = *static_cast<csrc_tracker*>(self);
    return reinterpret_cast<Py_hash_t&>(st);
}

int csrc_tracker_init(PyObject* self, PyObject* args, PyObject*) {
    unsigned line;
    unsigned col;
    if (!PyArg_ParseTuple(args, "II", &line, &col))
        return -1;
    new(&(static_cast<csrc_tracker*>(self)->st)) SrcLocation{line, col};
    return 0;
}

PyTypeObject csrc_trackerType {
        PyVarObject_HEAD_INIT(NULL, 0)
        "csrc_tracker.SrcLocation", /* tp_name */
        sizeof(csrc_tracker),      /* tp_basicsize */
        0,                         /* tp_itemsize */
        call_destructor<csrc_tracker>, /* tp_dealloc */
        0,                         /* tp_print */
        csrc_tracker_getattr,             /* tp_getattr */
        0,                         /* tp_setattr */
        0,                         /* tp_reserved */
        csrc_tracker_repr,                /* tp_repr */
        0,                         /* tp_as_number */
        0,                         /* tp_as_sequence */
        0,                         /* tp_as_mapping */
        csrc_tracker_hash,                /* tp_hash  */
        0,                         /* tp_call */
        0,                         /* tp_str */
        0,                         /* tp_getattro */
        0,                         /* tp_setattro */
        0,                         /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,        /* tp_flags */
        0,                         /* tp_doc */
        0,                         /* tp_traverse */
        0,                         /* tp_clear */
        csrc_tracker_cmp,                 /* tp_richcompare */
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
        csrc_tracker_init,                /* tp_init */
        0,                         /* tp_alloc */
        PyType_GenericNew          /* tp_new */
};

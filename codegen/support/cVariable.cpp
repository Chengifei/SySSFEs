//
// Created by Yifei Zheng on 09/09/2017.
//

#include <new>
#include "cVariable.hpp"
#include "cTypes.hpp"

PyObject* cVariable_getattr(PyObject* self, char* attr_name) {
    if (strcmp(attr_name, "type") == 0) {
        // This relies on cTypes being a immutable Python type.
        cTypes* obj = static_cast<cTypes*>(cTypesType.tp_new(&cTypesType, nullptr, nullptr));
        new(&obj->type) Types(static_cast<cVariable*>(self)->var.t);
        return obj;
    }
    else if (strcmp(attr_name, "val") == 0)
        return static_cast<cVariable*>(self)->val;
    PyErr_Format(PyExc_AttributeError,
                 "cVariable object has no attribute '%.400s'", attr_name);
    return nullptr;
}

int cVariable_setattr(PyObject* self, char* attr_name, PyObject* val) {
    if (strcmp(attr_name, "val")) {
        PyErr_Format(PyExc_AttributeError,
                     "Setting attribute '%.400s' is unsupported", attr_name);
        return -1;
    }
    static_cast<cVariable*>(self)->val = val;
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
    new(&(static_cast<cVariable*>(self)->var)) Variable{static_cast<cTypes*>(type)->type};
    Py_INCREF(Py_None);
    static_cast<cVariable*>(self)->val = val;
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

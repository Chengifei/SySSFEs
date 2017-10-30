//
// Created by Yifei Zheng on 09/09/2017.
//

#ifndef ATOM_CODEGEN_SUPPORT_CVARIABLE_HPP
#define ATOM_CODEGEN_SUPPORT_CVARIABLE_HPP
#include "../../common/python_common.hpp"
#include "Variable.hpp"
struct cVariable : PyObject {
    Variable var;
    PyObject* val;
};

PyObject* cVariable_getattr(PyObject* self, char* attr_name);
int cVariable_setattr(PyObject* self, char* attr_name, PyObject* val);
PyObject* cVariable_repr(PyObject* self);
int cVariable_init(PyObject* self, PyObject* args, PyObject*);

extern PyTypeObject cVariableType;
#endif

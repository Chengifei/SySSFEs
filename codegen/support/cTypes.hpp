//
// Created by Yifei Zheng on 08/09/2017.
//

#ifndef ATOMS_CODEGEN_SUPPORT_CTYPES_HPP
#define ATOMS_CODEGEN_SUPPORT_CTYPES_HPP
#include "../../common/python_common.hpp"
#include "Types.hpp"
struct cTypes : PyObject {
    Types type;
};

typedef PyObject BaseEnum;

PyObject* cTypes_cmp(PyObject* self, PyObject* rhs, int op);
PyObject* cTypes_getattr(PyObject* self, char* attr_name);
PyObject* cTypes_repr(PyObject* self);
Py_hash_t cTypes_hash(PyObject* self);
int cTypes_init(PyObject* self, PyObject* args, PyObject*);

extern PyTypeObject cTypesType;
extern PyTypeObject cTypesBaseEnum;
#endif

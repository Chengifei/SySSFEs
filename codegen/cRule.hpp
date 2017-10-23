/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * {Description}
 */
#include "../common/python_common.hpp"
#include "rule_types.hpp"
#include "support/SrcLocation.hpp"

struct cRule : PyObject {
    SrcLocation src;
    PyObject* dom;
    PyObject* lhs, *rhs;
};

PyObject* cRule_getattr(PyObject* self, char* attr_name);
PyObject* cRule_repr(PyObject* self);
int cRule_init(PyObject* self, PyObject* args, PyObject*);

extern PyTypeObject cRuleType;

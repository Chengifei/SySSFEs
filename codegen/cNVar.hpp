#include "../common/python_common.hpp"
#include "rule_types.hpp"

struct cNVar : PyObject {
    NVar var;
};

PyObject* cNVar_cmp(PyObject* self, PyObject* rhs, int op);
PyObject* cNVar_getattr(PyObject* self, char* attr_name);
PyObject* cNVar_repr(PyObject* self);
Py_hash_t cNVar_hash(PyObject* self);
int cNVar_init(PyObject* self, PyObject* args, PyObject*);

extern PyTypeObject cNVarType;

template <class... T>
PyObject* make_NVar(T&&... args) {
    cNVar* inst = static_cast<cNVar*>(cNVarType.tp_alloc(&cNVarType, 0));
    new(&inst->var) NVar(std::forward<T>(args)...);
    return inst;
}


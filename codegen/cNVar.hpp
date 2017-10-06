#include "../common/python_common.hpp"
#include "rule_types.hpp"

struct cNVar : PyObject, Variable {
    const char* name() const {
        return static_cast<const Variable*>(this)->name().c_str();
    }
    void need_update(bool flag) {
        _need_update = flag;
    }
    void can_start(bool flag) {
        _can_start = flag;
    }
};

int cNVar_init(PyObject* self, PyObject* args, PyObject*);
extern PyTypeObject cNVarType;

template <class... T>
PyObject* make_NVar(T&&... args) {
    cNVar* inst = static_cast<cNVar*>(cNVarType.tp_alloc(&cNVarType, 0));
    new(static_cast<Variable*>(inst)) Variable(std::forward<T>(args)...);
    return inst;
}


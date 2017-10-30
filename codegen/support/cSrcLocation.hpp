#include "../../common/python_common.hpp"
#include <utility>
#include <new>
#include "SrcLocation.hpp"

struct csrc_tracker : PyObject {
    SrcLocation st;
};

PyObject* csrc_tracker_cmp(PyObject* self, PyObject* rhs, int op);
PyObject* csrc_tracker_getattr(PyObject* self, char* attr_name);
PyObject* csrc_tracker_repr(PyObject* self);
Py_hash_t csrc_tracker_hash(PyObject* self);
int csrc_tracker_init(PyObject* self, PyObject* args, PyObject*);

extern PyTypeObject csrc_trackerType;
template <class... T>
PyObject* make_csrc_tracker(T&&... args) {
    csrc_tracker* inst = static_cast<csrc_tracker*>(csrc_trackerType.tp_alloc(&csrc_trackerType, 0));
    new(&inst->st) SrcLocation(std::forward<T>(args)...);
    return inst;
}

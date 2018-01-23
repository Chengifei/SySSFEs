/* Copyright 2017-2018 by Yifei Zheng
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <python_common.hpp>
#include <utility>
#include <new>
#include <support/src_location.hpp>

struct csrc_tracker : PyObject {
    support::src_location st;
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
    new(&inst->st) support::src_location(std::forward<T>(args)...);
    return inst;
}

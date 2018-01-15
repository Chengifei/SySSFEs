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
 *
 * Provides common python support routines packaged in RAII and exceptions.
 */

#ifndef PYTHON_COMMON_HPP
#define PYTHON_COMMON_HPP
#include "Python.h"
#include "frameobject.h"

class Python_API_Exception {};

class PyScoped {
    PyObject* ptr;
public:
    explicit PyScoped(PyObject* ptr) : ptr(ptr) {}
    PyScoped(const PyScoped&) = delete;
    PyScoped& operator=(const PyScoped&) = delete;
    PyScoped(PyScoped&& rhs) noexcept {
        ptr = rhs.ptr;
        rhs.ptr = nullptr;
    }
    PyScoped& operator=(PyScoped&& rhs) noexcept {
        Py_XDECREF(ptr);
        ptr = rhs.ptr;
        rhs.ptr = nullptr;
        return *this;
    }
    operator PyObject*() const {
        return ptr;
    }
    PyObject* get() const {
        return ptr;
    }
    ~PyScoped() noexcept {
        Py_XDECREF(ptr);
    }
};

template <class T>
void call_destructor(PyObject* self) noexcept {
    reinterpret_cast<T*>(self)->~T();
    (*Py_TYPE(self)->tp_free)(self);
}

static PyObject* inc_prpg(PyObject* obj) noexcept {
    Py_INCREF(obj);
    return obj;
}

template <class T, class U>
T PyExc(T t, U fail_ret) {
    if (t == fail_ret)
        throw Python_API_Exception();
    return t;
}

template <class T, class U>
T PyOnly(T t, U good_ret) {
    if (t != good_ret)
        throw Python_API_Exception();
    return t;
}

static void PyTraceback(const char* funcname, const char* filename, unsigned lineno) {
    PyObject* type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    PyScoped globals(PyExc(PyDict_New(), nullptr));
    PyCodeObject* code(PyExc(PyCode_NewEmpty(filename, funcname, lineno), nullptr));
    PyScoped code_(reinterpret_cast<PyObject*>(code));
    PyFrameObject* frame(PyExc(PyFrame_New(PyThreadState_Get(), code, globals, nullptr), nullptr));
    PyScoped frame_(reinterpret_cast<PyObject*>(frame));
    frame->f_lineno = lineno;
    PyTracebackObject* tb(PyExc(PyObject_GC_New(PyTracebackObject, &PyTraceBack_Type), nullptr));
    PyObject* tb_(reinterpret_cast<PyObject*>(tb));
    tb->tb_next = nullptr;
    Py_INCREF(frame);
    tb->tb_frame = frame;
    tb->tb_lasti = frame->f_lasti;
    tb->tb_lineno = lineno;
    PyObject_GC_Track(tb);
    PyErr_Restore(type, value, tb_);
}
#endif

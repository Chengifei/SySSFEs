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

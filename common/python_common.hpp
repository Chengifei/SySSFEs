#ifndef PYTHON_COMMON_HPP
#define PYTHON_COMMON_HPP
#include "Python.h"

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
T PyOnly(T t, U fail_ret) {
    if (t != fail_ret)
        throw Python_API_Exception();
    return t;
}
#endif

#include "math_utils.hpp"
#include <cassert>
using std::string;
#ifdef PY_EXT
map<string, double> _PyDict_to_UMap(PyObject* dict) {
    assert(PyDict_Check(dict));
    Py_ssize_t n = 0;
    auto nptr = &n;
    PyObject *key, *value;
    map<string, double> result;
    while (PyDict_Next(dict, nptr, &key, &value)) {
        result.insert({ PyUnicode_AsUTF8(key), PyFloat_AsDouble(value) });
    }
    return result;
}
#endif

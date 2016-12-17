#ifndef MATH_UTILS_HPP
#define MATH_UTILS_HPP
#ifdef PY_EXT
#include "Python.h"
#endif
#include <cfloat>
#include <cmath>
#include <algorithm>
#include "phycomap.hpp"

//! Cast a c-str to `unsigned int`
constexpr unsigned str2int(const char* str, int h = 0) {
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

#ifdef PY_EXT
map<std::string, double> _PyDict_to_UMap(PyObject*);
#endif

template <typename T>
bool _fp_almost_equal(T A, T B, T max_abs_diff, T max_rel_diff) {
    // Check if the numbers are really close -- needed
    // when comparing numbers near zero.
    T diff = fabs(A - B);
    if (diff <= max_abs_diff)
        return true;
    A = fabs(A);
    B = fabs(B);
    if (diff <= std::max(A, B) * max_rel_diff)
        return true;
    return false;
}

template <typename T>
bool _fp_almost_equal_abs_only(T A, T B, T max_abs_diff) {
    return fabs(A - B) <= max_abs_diff;
}

inline bool double_equal(double A, double B, double max_abs_diff = DBL_EPSILON, double max_rel_diff = DBL_EPSILON) {
    return _fp_almost_equal(A, B, max_abs_diff, max_rel_diff);
}

inline bool double_equal_abs_only(double A, double B, double max_abs_diff = DBL_EPSILON) {
    return _fp_almost_equal_abs_only(A, B, max_abs_diff);
}

template <typename T>
int signum(T t) {
    if (t > 0)
        return 1;
    else if (t == 0)
        return 0;
    else
        return -1;
}

template <typename T>
int fsignum(T t, T max_abs_diff) {
    if (_fp_almost_equal_abs_only<T>(t, 0, max_abs_diff))
        return 0;
    else if (t > 0)
        return 1;
    else
        return -1;
}
#endif


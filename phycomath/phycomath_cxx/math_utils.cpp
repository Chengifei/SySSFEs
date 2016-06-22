/*! \file */
#ifndef MATH_UTILS_CPP
#define MATH_UTILS_CPP
#include <string>
using std::string;

//!
//! Check if `str` is of [a-zA-Z]
//!
inline bool is_alpha(const string& str) noexcept {
    for (char i : str) {
        if (!(('a' <= i  && i <= 'z') || ('A' <= i && i <= 'Z')))
            return false;
    }
    return true;
}

// There are strong evidence that shows manually written function are
// dramatically faster than it by regex.
//!
//! Check if str can be converted to double or int
//!
inline int is_double(const string& str) noexcept {
    unsigned dotcount = 0;
    for (char i : str) {
        if ('0' <= i && i <= '9') {
        }
        else if (i == '.')
            ++dotcount;
        else //failure not dot or digit
            return 0;
    }
    switch (dotcount) {
        case 1:
            return 1;
        case 0:
            return 2;
        default:
            return 0;
    }
}

//!
//! Cast a c-str to `unsigned int`
//!
constexpr unsigned str2int(const char* str, int h = 0) {
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

//!
//! Apply a filter `pred` to a STL container, append it into a new container
//! if `pred::test` returns false, finally return the new container.
//!
template <class T, class pred>
T filter(T& t) {
    T ret;
    for (auto it = t.begin(); it != t.end();)
        if (!pred::test(*it)) {
            ret.push_back(*it);
            it = t.erase(it);
        }
        else
            ++it;
    return ret;
}
#endif


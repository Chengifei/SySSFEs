/*
Copyright 2016 by Yifei Zheng
All rights reserved.
*/
#ifndef OPERATORS_HPP
#define OPERATORS_HPP
#include <cmath>
#include <string>
#include <set>
using std::string;
#ifdef PHYCO_USE_UMAP
#include <unordered_map>
#else
#include <map>
#define unordered_map map
#endif
namespace math {

struct Op {
    class func_t {
        using funcptr = double (*)(const std::vector<double>&);
        funcptr fptr;
    public:
        func_t(funcptr fptr) noexcept : fptr(fptr) {};
        func_t& operator=(const func_t&) noexcept = default;
        func_t(const func_t&) noexcept = default;
        double operator()(const std::vector<double>& a) const noexcept {
            return fptr(a);
        }
    };
public: // bad practice
    const string name;
    const unsigned priority;
    const func_t func;
private:
    enum { NONE, LEFT, RIGHT } const massoc = NONE;
    const bool minfix;
public:
    Op(string, unsigned, func_t, bool infix = true);
    Op(Op&& other) = delete;
    Op& operator=(Op&& other)=delete;
    Op& operator=(const Op&) = delete;
    Op(const Op&) = delete;
    bool operator==(const string& str) const noexcept {
        return str == name;
    }
    bool operator==(const Op& rhs) const noexcept {
        return name == rhs.name;
    }
    int assoc() const noexcept {
        return massoc;
    }
    bool infix() const noexcept {
        return minfix;
    }
#ifdef PY_EXT
    string repr() const noexcept;
#endif
};

namespace operators {
namespace funcs {

inline double plus(const std::vector<double>& a) noexcept {
    double result = 0;
    for (auto i : a)
        result += i;
    return result;
}
inline double minus(const std::vector<double>& a) noexcept {
    double result = a.front();
    for (auto it = ++a.begin(); it != a.end(); ++it)
        result -= *it;
    return result;
}
inline double mul(const std::vector<double>& a) noexcept {
    double result = a.front();
    for (auto it = ++a.begin(); it != a.end(); ++it)
        result *= *it;
    return result;
}
inline double div(const std::vector<double>& a) noexcept {
    double result = a.front();
    for (auto it = ++a.begin(); it != a.end(); ++it)
        result /= *it;
    return result;
}

inline double pow_wrap(const std::vector<double>& a) noexcept {
    return pow(a.front(), *++a.begin());
}

inline double sin_wrap(const std::vector<double>& a) noexcept {
    return sin(a.front());
}

inline double cos_wrap(const std::vector<double>& a) noexcept {
    return cos(a.front());
}

inline double tan_wrap(const std::vector<double>& a) noexcept {
    return tan(a.front());
}

inline double ln_wrap(const std::vector<double>& a) noexcept {
    return log(a.front());
}

inline double exp_wrap(const std::vector<double>& a) noexcept {
    return exp(a.front());
}

inline double sqrt_wrap(const std::vector<double>& a) noexcept {
    return sqrt(a.front());
}
}

const Op PLU_OP("+", 1, &funcs::plus);
const Op MIN_OP("-", 1, &funcs::minus);
const Op MUL_OP("*", 2, &funcs::mul);
const Op DIV_OP("/", 2, &funcs::div);
const Op POW_OP("^", 3, &funcs::pow_wrap);
const Op SIN_OP("sin", 0, &funcs::sin_wrap, false);
const Op COS_OP("cos", 0, &funcs::cos_wrap, false);
const Op TAN_OP("tan", 0, &funcs::tan_wrap, false);
const Op LN_OP("ln", 0, &funcs::ln_wrap, false);
const Op EXP_OP("exp", 0, &funcs::exp_wrap, false);
const Op SQRT_OP("sqrt", 0, &funcs::sqrt_wrap, false);

typedef std::unordered_map<string, const Op*> Opdict;
extern std::set<string> infixlist, bilist;

extern const Opdict infix_dict, built_in_dict;

void init() noexcept;

}
}
#endif

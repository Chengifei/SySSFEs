/*
Copyright 2016 by Yifei Zheng
All rights reserved.
*/
#ifndef OPERATORS_HPP
#define OPERATORS_HPP
#include <cmath>
#include <functional>
#include <string>
using std::string;
#ifdef PHYCO_USE_UMAP
#include <unordered_map>
#else
#include <map>
#define unordered_map map
#endif
namespace math {

struct Op {
    class binfunc {
        enum :bool { UN, BIN } tag;
        union {
            double(*bptr)(double, double);
            double(*uptr)(double);
        };
    public:
        binfunc(double(*fptr)(double, double)) noexcept : tag(BIN), bptr(fptr) {};
        binfunc(double(*fptr)(double)) noexcept :tag(UN), uptr(fptr) {};
        binfunc& operator=(const binfunc&) noexcept = default;
        binfunc(const binfunc&) noexcept = default;
        binfunc(binfunc&& other) noexcept : tag(other.tag) {
            if (tag)
                uptr=other.uptr;
            else
                bptr=other.bptr;
        }
        binfunc& operator=(binfunc&& other) noexcept {
            tag=other.tag;
            if (tag)
                uptr=other.uptr;
            else
                bptr=other.bptr;
            return *this;
        }
        inline double operator()(double a, double b) const noexcept {
            return tag ? bptr(a, b) : uptr(a);
        }
    };
public: // bad practice
    const string name;
    const unsigned priority;
    const binfunc func;
private:
    enum { NONE, LEFT, RIGHT } const massoc = NONE;
    const bool minfix;
public:
    Op(string, unsigned, binfunc, bool infix = true);
    Op(Op&& other) = delete;
    Op& operator=(Op&& other)=delete;
    Op& operator=(const Op&) = delete;
    Op(const Op&) = delete;
    bool operator==(string str) const noexcept {
        return str == name;
    }
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

inline double plus(double a, double b) noexcept {
    return a + b;
}
inline double minus(double a, double b) noexcept {
    return a - b;
}
inline double mul(double a, double b) noexcept {
    return a * b;
}
inline double div(double a, double b) noexcept {
    return a / b;
}

}

const Op PLU_OP("+", 1, &funcs::plus);
const Op MIN_OP("-", 1, &funcs::minus);
const Op MUL_OP("*", 2, &funcs::mul);
const Op DIV_OP("/", 2, &funcs::div);
const Op POW_OP("^", 3, &pow);
const Op SIN_OP("sin", 4, &sin, false);
const Op COS_OP("cos", 0, &cos, false);
const Op TAN_OP("tan", 0, &tan, false);
const Op LN_OP("ln", 0, &log, false);
const Op EXP_OP("exp", 0, &exp, false);
const Op SQRT_OP("sqrt", 0, &sqrt, false);

typedef std::unordered_map<string, const Op*> Opdict;
extern std::set<string> infixlist, bilist;

extern const Opdict infix_dict, built_in_dict;

void init() noexcept;

}
}
#endif

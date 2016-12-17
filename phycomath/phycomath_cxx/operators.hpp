/*
Copyright 2016 by Yifei Zheng
All rights reserved.
*/
#ifndef OPERATORS_HPP
#define OPERATORS_HPP
#include <string>
#include <set>
#include "phycomap.hpp"
#include "gen.hpp"
using std::string;

namespace math {

struct Op {
public: // bad practice
    using func_t = double (*)(utils::vgen_base<double>&);
    const string name;
    const unsigned priority;
    const func_t func;
private:
    enum { NONE, LEFT, RIGHT } const massoc = NONE;
    const bool minfix;
public:
    Op(string, unsigned, func_t, bool infix = true);
    Op(Op&& other) = delete;
    Op& operator=(Op&& other) = delete;
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
    double operator()(utils::vgen_base<double>& a) const noexcept {
        return func(a);
    }
#ifdef PY_EXT
    string repr() const noexcept;
#endif
};

namespace operators {
extern const Op PLU_OP, MIN_OP, MUL_OP, DIV_OP, POW_OP, SIN_OP, COS_OP, TAN_OP;
extern const Op LN_OP, EXP_OP, SQRT_OP;
extern std::set<string> infixlist, bilist;
extern const map<string, const Op*> infix_dict, built_in_dict;

void init() noexcept;

}

}
#endif

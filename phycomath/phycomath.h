#pragma once
#ifndef PHYCO_MATH_H
#define PHYCO_MATH_H
#define PY_EXT
#include "phycoparse.h"
#include <unordered_map>
#include <queue>
#include <stack>
#include <cmath>
#include <functional>
#include <memory>
#include <boost/variant.hpp>

namespace math {
struct Op {
    typedef std::function<double(double, double)> binfunc;
    typedef std::function<double(double)> unfunc;
private:
    auto wrapper(unfunc);
public:
    string name;
    unsigned priority;
    bool infix;
    binfunc func;
    Op(string, unsigned, binfunc, bool infix = true);
    Op(string, unsigned, unfunc, bool infix = true);
#ifdef PY_EXT
    string repr() const;
#endif
};
namespace operators {
namespace funcs {

static double plus(double a, double b) {
    return a + b;
}
static double minus(double a, double b) {
    return a - b;
}
static double mul(double a, double b) {
    return a * b;
}
static double div(double a, double b) {
    return a / b;
}

}

const Op PLU_OP("+", 1, (Op::binfunc) &funcs::plus);
const Op MIN_OP("-", 1, (Op::binfunc) &funcs::minus);
const Op MUL_OP("*", 2, (Op::binfunc) &funcs::mul);
const Op DIV_OP("/", 2, (Op::binfunc) &funcs::div);
#ifdef _MSC_VER
const Op POW_OP("^", 3, (Op::binfunc) &std::powl);
const Op SIN_OP("sin", 4, (Op::unfunc) &std::sinl, false);
#else
const Op POW_OP("^", 3, (math::Op::binfunc) &pow);
const Op SIN_OP("sin", 4, (math::Op::unfunc) &sin);
#endif

typedef std::unordered_map<string, Op> Opdict;
extern set<string> infixlist;
extern set<string> bilist;

extern const Opdict infix_dict;
extern const Opdict built_in_dict;

void init();
}

struct plexpr;
typedef std::shared_ptr<plexpr> expr_ptr;
typedef boost::variant<double, int, string, expr_ptr, decltype(nullptr)> arg_t;

struct plexpr {
private:
    enum req_t { op_ = 0, arg1_ = 1, arg2_ = 2 };
public:
    req_t req;
    std::stack<expr_ptr> brackets;
    Op op;
    arg_t arg1;
    arg_t arg2;
    //std::pair<expr_ptr, unsigned> bottom;
    plexpr(Op op = operators::PLU_OP, arg_t arg1 = 0, arg_t arg2 = nullptr);
    plexpr(expr_ptr);
    double exec();
    void append(int);
    void append(double);
    void append(Op&);
    void append(string);
    void appendL();
    void appendR();
    void appendBi(Op&);
    void appendvar(string);
    bool sequal(expr_ptr);
};

plexpr parse(string);

namespace simplify {
typedef boost::variant<plexpr, int, string, double> simpr_t;
typedef std::function<bool(plexpr)> cond_t;
typedef std::function<simpr_t(plexpr)> to_t;
struct conv_rule : public boost::static_visitor<bool> {
    struct simpmatch_visitor : public boost::static_visitor<bool> {
        cond_t cond;
        simpmatch_visitor(cond_t cond) :cond(cond) {

        }
        template <class T>
        bool operator()(T arg) const {
            return false;
        }
        bool operator()(plexpr arg) const {
            return cond(arg);
        }
    };
    simpmatch_visitor visitor;
    to_t to;
    conv_rule(cond_t, to_t);
    bool match(simpr_t) const;
    simpr_t apply(plexpr) const;
};
simpr_t simplify(plexpr);
}

namespace convert {
string Arg2str(arg_t);
string expr2str(plexpr);
string simp2str(simplify::simpr_t);
}

}
#endif

#include "operators.hpp"
#include <cmath>

namespace math::operators {
namespace funcs {
#define OP(name) inline double name(utils::vgen_base<double>& g) noexcept
OP(plus) {
    double result = 0;
    for (; g.valid(); ++g)
        result += g.get();
    return result;
}
OP(minus) {
    double result = g.get();
    ++g;
    for (; g.valid(); ++g)
        result -= g.get();
    return result;
}
OP(mul) {
    double result = g.get();
    ++g;
    for (; g.valid(); ++g)
        result *= g.get();
    return result;
}
OP(div) {
    double result = g.get();
    ++g;
    for (; g.valid(); ++g)
        result /= g.get();
    return result;
}

OP(pow_wrap) {
    double base = g.get();
    ++g;
    double exp = g.get();
    return pow(base, exp);
}

OP(sin_wrap) {
    return sin(g.get());
}

OP(cos_wrap) {
    return cos(g.get());
}

OP(tan_wrap) {
    return tan(g.get());
}

OP(ln_wrap) {
    return log(g.get());
}

OP(exp_wrap) {
    return exp(g.get());
}

OP(sqrt_wrap) {
    return sqrt(g.get());
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

}

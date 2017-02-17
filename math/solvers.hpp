#ifndef MATH_SOLVER_HPP
#define MATH_SOLVER_HPP
#include <cmath>
#include <stdexcept>
#include <limits>

namespace math::solver {

template <class Eval, class Env>
double fdiff(const Eval& expr, double& x, Env& env) {
    double step = (fabs(x) > 1) ? std::numeric_limits<double>::epsilon() * x :
                                  std::numeric_limits<double>::epsilon();
    x -= step;
    auto y1 = expr.template eval<double>(env);
    x += step + step;
    auto y2 = expr.template eval<double>(env);
    x -= step;
    return (y2 - y1) / (step + step);
}

template <class Eval, class Env>
void fNsolve(const Eval& expr, double& x, Env& env) {
    for (unsigned i = 0; i != 100; ++i) {
        double diff = expr.template eval<double>(env);
        if (fabs(diff) > 0x1p-20)
            x -= diff / fdiff(expr, x, env);
        else
            return;
    }
    if (fabs(expr.template eval<double>(env)) > 0x1p-20)
        throw std::runtime_error("Newton's method not converging, simulation failed");
}


template <class Eval, class Env>
void buildJcb(const Eval& expr, double& x, Env& env) {
    for (unsigned i = 0; i != 100; ++i) {
        double diff = expr.template eval<double>(env);
        if (fabs(diff) > 0x1p-20)
            x -= diff / fdiff(expr, x, env);
        else
            return;
    }
    if (fabs(expr.template eval<double>(env)) > 0x1p-20)
        throw std::runtime_error("Newton's method not converging, simulation failed");
}

}
#endif

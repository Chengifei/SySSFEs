#ifndef MATH_SOLVER_HPP
#define MATH_SOLVER_HPP
#include <cmath>
#include <stdexcept>
#include <limits>
#include "Eigen/Dense"
#include <initializer_list>
#include "calculus.hpp"

namespace math::solver {

template <class F>
double algebraic_single(const F& expr, double x) {
    double diff = expr(x);
    double tol = (fabs(diff) > 1) ? 0x1p-27 * diff :
                                    0x1p-27;
    for (unsigned i = 0; i != 32; ++i) {
        if (fabs(diff) < tol)
            return x;
        x -= diff * rdiff(expr, x);
        diff = expr(x);
    }
    throw std::runtime_error("Newton's iteration not converging");
}

template <class F1, class F2, class R>
double differential_single(const F1& lhs, const F2& rhs, const R& record_gen) {
    auto derivative = [&lhs, &rhs](double t, double x) { return rhs(t, x) * rdiff(lhs, x); };
    auto last = record_gen(1);
    auto last_2 = record_gen(2);
    double step = last.first - last_2.first;
    double x = algebraic_single([&derivative, &last, &last_2, step](double n) {
        return n - (last.second + step * 0.5 * (derivative(last.first, last.second) + derivative(last_2.first, last_2.second)));
    }, last.second);
    return x;
}

void buildJcb(
Eigen::MatrixXd& mat, const std::initializer_list<double*>& vars, unsigned i) {}

template <class F, class... FRest>
void buildJcb(
Eigen::MatrixXd& mat, const std::initializer_list<double*>& vars, unsigned i, const F& expr,
const FRest&... FRests) {
    for (unsigned j = 0; j != vars.size(); ++j) {
        double x_ = *vars.begin()[j];
        mat(i, j) = fdiff([&expr, &vars, &xpos = *vars.begin()[j]](double x){
                                xpos = x;
                                return expr(vars);
                            }, *vars.begin()[j]);
        *vars.begin()[j] = x_;
    }
    buildJcb(mat, vars, i + 1, FRests...);
}

void buildDiff(
Eigen::VectorXd& rhs, const std::initializer_list<double*>& vars, unsigned i) {}

template <class F, class... FRest>
void buildDiff(Eigen::VectorXd& rhs, const std::initializer_list<double*>& vars, unsigned i, const F& expr, const FRest&... FRests) {
    rhs(i) = expr(vars);
    buildDiff(rhs, vars, i + 1, FRests...);
}

template <class... Fs>
void algebraic_sys(const std::initializer_list<double*>& vars, const Fs&... exprs) {
    for (unsigned i = 0; i != 100; ++i) {
        Eigen::MatrixXd jacobian(vars.size(), vars.size());
        buildJcb(jacobian, vars, 0, exprs...);
        Eigen::VectorXd rhs(vars.size());
        buildDiff(rhs, vars, 0, exprs...);
        Eigen::VectorXd result = jacobian.colPivHouseholderQr().solve(rhs);
        for (unsigned i = 0; i != vars.size(); ++i)
            *vars.begin()[i] -= result(i);
    }
}

}
#endif

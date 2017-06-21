#ifndef MATH_SOLVER_HPP
#define MATH_SOLVER_HPP
#include <cmath>
#include <utility>
#include <stdexcept>
#include <limits>
#include "Eigen/Dense"
#include <initializer_list>
#include "calculus.hpp"

namespace math::solver {

template <class F>
double algebraic_single(const F& expr, double x) {
    double diff = expr(x);
    double tol = (fabs(diff) > 1) ? 0x1p-27 * fabs(diff) :
                                    0x1p-27;
    for (unsigned i = 0; i != 32; ++i) {
        if (fabs(diff) < tol)
            return x;
        x -= diff * calculus::rdiff(expr, x);
        diff = expr(x);
    }
    throw std::runtime_error("Newton's iteration not converging");
}

template <class F>
std::pair<double, double>
// As tested on g++ 7.1, it's able to optimize if only part of pair is used.
differential(const F& derivative, double t, double x, double step) {
    double k = derivative(t + step * 0.5, x + step * 0.5 * derivative(t, x));
    double n = x + step * k;
    return {n, k};
}

void buildJcb(
Eigen::MatrixXd& mat, const std::initializer_list<double*>& vars, unsigned i) {}

template <class F, class... FRest>
void buildJcb(
Eigen::MatrixXd& mat, const std::initializer_list<double*>& vars, unsigned i, const F& expr,
const FRest&... FRests) {
    for (unsigned j = 0; j != vars.size(); ++j) {
        double x_ = *vars.begin()[j];
        mat(i, j) = calculus::fdiff([&expr, &vars, &xpos = *vars.begin()[j]](double x){
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

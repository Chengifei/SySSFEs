#ifndef MATH_SOLVER_HPP
#define MATH_SOLVER_HPP
#include <cmath>
#include <stdexcept>
#include <limits>
#include "Eigen/Dense"
#include <vector>

namespace math::solver {

template <class F>
double fdiff(const F& expr, double x) {
    double step = (fabs(x) > 1) ? 0x1p-20 * x :
                                  0x1p-20;
    auto y1 = expr(x - step);
    auto y2 = expr(x + step);
    return (y2 - y1) / (step + step);
}

template <class F>
double rdiff(const F& expr, double x) {
    double step = (fabs(x) > 1) ? 0x1p-20 * x :
                                  0x1p-20;
    auto y1 = expr(x - step);
    auto y2 = expr(x + step);
    return (step + step) / (y2 - y1);
}

template <class F>
double solve_algebraic_single(const F& expr, double x) {
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

template <class F1, class F2>
double solve_differential_single(const F1& lhs, const F2& rhs, double step, double& x) {
    double x_ = x;
    auto derivative = [&lhs, &rhs](double x) { return rhs(x) * rdiff(lhs, x); };
    solve_algebraic_single([&derivative, x, step](double n) {
                            return n - (x + step * derivative((x + n) * 0.5));
                            }, x);
    return (x - x_) / step;
}

void buildJcb(
Eigen::MatrixXd& mat, const std::vector<double*>& vars, unsigned i) {}

template <class F, class... FRest>
void buildJcb(
Eigen::MatrixXd& mat, const std::vector<double*>& vars, unsigned i, const F& expr,
const FRest&... FRests) {
    for (unsigned j = 0; j != vars.size(); ++j) {
        double x_ = *vars[j];
        mat(i, j) = fdiff([&expr, &vars, &xpos = *vars[j]](double x){
                                xpos = x;
                                return expr(vars);
                            }, *vars[j]);
        *vars[j] = x_;
    }
    buildJcb(mat, vars, i + 1, FRests...);
}

void buildDiff(
Eigen::VectorXd& rhs, const std::vector<double*>& vars, unsigned i) {}

template <class F, class... FRest>
void buildDiff(Eigen::VectorXd& rhs, const std::vector<double*>& vars, unsigned i, const F& expr, const FRest&... FRests) {
    rhs(i) = expr(vars);
    buildDiff(rhs, vars, i + 1, FRests...);
}

template <class... Fs>
void solve_algebraic_sys(const std::vector<double*>& vars, const Fs&... exprs) {
    for (unsigned i = 0; i != 100; ++i) {
        Eigen::MatrixXd jacobian(vars.size(), vars.size());
        buildJcb(jacobian, vars, 0, exprs...);
        Eigen::VectorXd rhs(vars.size());
        buildDiff(rhs, vars, 0, exprs...);
        Eigen::VectorXd result = jacobian.colPivHouseholderQr().solve(rhs);
        for (unsigned i = 0; i != vars.size(); ++i)
            *vars[i] -= result(i);
    }
}

}
#endif

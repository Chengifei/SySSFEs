#include "solver.hpp"
#include "math_utils.hpp"
#include "expression_tree.hpp"
#include "calculus.hpp"

namespace math::solver {

std::set<string> count_all_vars(const expression_tree& expr) {
    std::set<string> ret;
    for (auto& node : expr.citer()) {
        if (auto ptr = dynamic_cast<const string_wrapper* const>(node.content.get_ptr()))
            ret.insert(ptr->to_string());
    }
    return ret;
}

#define TOLERANCE 1e-7
#define MAX_ITER 1000

double fNsolve(const expression_tree& expr, string varname, map<string, double> vardict) {
    double& x = vardict.at(varname);
    for (unsigned i = 0; i != MAX_ITER; ++i) {
        double diff = expr.fexec(vardict);
        if (fabs(diff) > TOLERANCE)
            x -= diff / calculus::fdiff(expr, varname, vardict);
        else
            break;
    }
    if (fabs(expr.fexec(vardict)) <= TOLERANCE)
        return x;
    throw std::runtime_error("Newton's method not converging (HINT: Try changing the seed)");
}

} // namespace solver

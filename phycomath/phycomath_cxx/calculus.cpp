#include "calculus.hpp"
#include <cfloat>
#include "expression_tree.hpp"

namespace math::calculus {

double fdiff(const expression_tree& expr, string varname, double x) {
    double step = (fabs(x) > 1) ? DBL_EPSILON * x : DBL_EPSILON;
    map<string, double>vardict{ {varname, x - step} };
    auto y1 = expr.fexec(vardict);
    vardict[varname] = x + step;
    auto y2 = expr.fexec(vardict);
    return (y2 - y1) / step / 2;
}

double fdiff(const expression_tree& expr, string varname, map<string, double> vardict) {
    double& val = vardict[varname];
    double step = (fabs(val) > 1) ? DBL_EPSILON * val : DBL_EPSILON;
    val -= step;
    auto y1 = expr.fexec(vardict);
    val += 2 * step;
    auto y2 = expr.fexec(vardict);
    return (y2 - y1) / step / 2;
}

}

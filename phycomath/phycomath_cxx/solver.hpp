#ifndef SOLVER_HPP
#define SOLVER_HPP
#include "math_fwd.hpp"
#include <set>
#include "phycomap.hpp"
#ifdef PY_EXT
#include "Python.h"
#endif

namespace math::solver {
double fNsolve(const expression_tree&, string varname, map<string, double> vardict = {});
std::set<string> count_all_vars(const expression_tree&);
#ifdef PY_EXT
#endif
}
#endif

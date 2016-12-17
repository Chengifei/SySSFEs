#ifndef PHYCO_MATH_H
#define PHYCO_MATH_H
#include "math_fwd.hpp"
#include "phycomap.hpp"

namespace math {

expression_tree parse(string);
expression_tree& eval(expression_tree&, const map<string, double>&);

}

#include "calculus.hpp"
#include "equation.hpp"
#include "solver.hpp"

#endif

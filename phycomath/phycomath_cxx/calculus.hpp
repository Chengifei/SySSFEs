#ifndef CALCULUS_HPP
#define CALCULUS_HPP
#include "phycomap.hpp"
#include "math_fwd.hpp"

namespace math::calculus {
double fdiff(const expression_tree&, string, double);
double fdiff(const expression_tree&, string, map<string, double>);
}
#endif

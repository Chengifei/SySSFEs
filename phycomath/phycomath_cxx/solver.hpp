#ifndef SOLVER_HPP
#define SOLVER_HPP
namespace solver {
double fNsolve(const expression_tree&, string varname, double seed, std::unordered_map<string, double> vardict);
}
#endif

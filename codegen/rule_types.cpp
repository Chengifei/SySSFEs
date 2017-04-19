#include "rule_types.hpp"

std::size_t hash_value(const NVar& self) {
    std::size_t hash = 14695981039346656037U;
    for (char i : self.name) {
        hash ^= i;
        hash *= 1099511628211U;
    }
    return hash & ~0b111 | self.order;
}

void ResolvingOrder::add_step(unsigned r, NVar v) {
    seq.emplace_back(new step<EqnSolver::ALG_S>(r, v));
}

void ResolvingOrder::add_step(std::vector<unsigned> rs, std::vector<NVar> vs) {
    seq.emplace_back(new step<EqnSolver::ALG_M>(rs, vs));
}

void ResolvingOrder::add_step(NVar var, unsigned to) {
    seq.emplace_back(new step<EqnSolver::DIFF_S>(var, NVar(var.name, to)));
}

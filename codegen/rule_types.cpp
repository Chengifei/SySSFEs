#include "rule_types.hpp"

std::size_t hash_value(const NVar& self) {
    std::size_t hash = 14695981039346656037U;
    for (char i : self.name) {
        hash ^= i;
        hash *= 1099511628211U;
    }
    return (hash & ~0b111) | *reinterpret_cast<const std::uint64_t*>(self.order.data());
}

void ResolvingOrder::add_alg(std::size_t r, NVar v) {
    seq.emplace_back(new step<EqnSolver::ALG_S>(r, std::move(v)));
}

void ResolvingOrder::add_alg_sys(std::vector<std::size_t> rs, std::vector<NVar> vs) {
    seq.emplace_back(new step<EqnSolver::ALG_M>(std::move(rs), std::move(vs)));
}

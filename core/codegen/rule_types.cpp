#include "rule_types.hpp"

std::size_t hash_value(const Variable& self) {
    std::size_t hash = 14695981039346656037U;
    for (char i : self.name()) {
        hash ^= i;
        hash *= 1099511628211U;
    }
    return (hash & ~0b111) | *reinterpret_cast<const std::uint64_t*>(self.order().data());
}

void ResolvingOrder::add_alg(Rule* r, Variable v) {
    seq.emplace_back(new step<false>(r, std::move(v)));
}

void ResolvingOrder::add_alg_sys(Rule* b, Rule* e, std::vector<Variable> vs) {
    seq.emplace_back(new step<true>(b, e, std::move(vs)));
}

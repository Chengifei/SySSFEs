/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This file implements routines that actually resolve the solving order.
 */

#include <utility>
#include <algorithm>
#include "iter_utils.hpp"
#include "rule_types.hpp"
#include "../common/typed_buffer.hpp"
#include <cassert>

struct eqn_try : iter_utils::non_trivial_end_iter<eqn_try> {
    typedef std::vector<Rule> eqn_container;
    typedef std::vector<NVar> var_container;
    typedef var_container::iterator var_iter;
    typedef combination<var_iter> comb_type;
    powerset<eqn_container> eqn_choice;
    var_container vars;
    placeholder<comb_type> selection;
    eqn_try(eqn_container& pack)
        : eqn_choice(pack, pack.size()) {
        for (const auto& eqn : *eqn_choice)
            std::copy(eqn.vars.begin(), eqn.vars.end(), std::back_inserter(vars));
        new(&selection) comb_type(vars.begin(), vars.end(), eqn_choice->size());
    }
    eqn_try& operator++() {
        ++*selection;
        if (selection->exhausted()) {
            vars.clear();
            do {
                ++eqn_choice;
                for (const auto &eqn : *eqn_choice)
                    std::copy(eqn.vars.begin(), eqn.vars.end(),
                              std::back_inserter(vars));
            } while (vars.size() < eqn_choice->size() && !eqn_choice.exhausted());
            assert(!eqn_choice.exhausted()); // TODO: handle this
            selection->~comb_type();
            new(&selection) comb_type(vars.begin(), vars.end(), eqn_choice->size());
        }
        return *this;
    }
    std::pair<std::vector<std::size_t>, const NVar*> operator*() {
        std::vector<std::size_t> ret;
        ret.reserve(eqn_choice->size());
        for (const auto& eqn : *eqn_choice)
            ret.push_back(eqn.unique_id);
        return std::make_pair(std::move(ret), **selection);
    }
    bool exhausted() {
        return eqn_choice.exhausted();
    }
    ~eqn_try() {
        selection->~comb_type();
    }
};

bool RuleResolver::validate_resolution() const noexcept {
    for (const Rule& eqn : pack)
        if (!eqn.unknowns.empty())
            return false;
    return true;
}

#ifndef	__cpp_deduction_guides
template <class T>
eqn_try<T> make_eqn_try(T b, T e, std::size_t sz) {
    return eqn_try<T>(b, e, sz);
}
#else
#define make_eqn_try eqn_try
#endif

ResolvingOrder RuleResolver::get() {
    return std::move(order);
}

bool RuleResolver::process() {
    // TODO: Independent starts selection shall be deterministic, then we can
    // even eliminate optionals.
    auto Pack = pack;
    if (!alg_consistent(std::vector<NVar>{}))
        return false;
    if (validate_resolution())
        return true;
    for (auto attempt : make_eqn_try(pack)) {
        unsigned step_count = 0;
        const NVar* starts = attempt.second;
        auto& occurrences = attempt.first;
        std::size_t size = occurrences.size();
        if (size == 1)
            order.add_alg(occurrences[0], *starts);
        else {
            assert(size != 0);
            order.add_alg_sys(std::move(occurrences),
                              std::vector<NVar>(starts,
                                                starts + occurrences.size()));
        }
        ++step_count;
        auto Pack = pack;
        // to keep consistency of the system:
        // 1. the derivatives of the same variable must all be updated. (broadcast)
        // 2. the last variable in an algebraic equation must be computed
        //    once all the other variables are known.
        for (const NVar& var : iter_utils::as_array(starts, size))
            if (!broadcast(var))
                break;
        if (std::optional<unsigned> a_s =
                    alg_consistent(iter_utils::as_array(starts, size)))
            step_count += *a_s;
        if (validate_resolution())
            return true;
        else {
            pack = std::move(Pack);
            order.clear();
        }
    }
    return false;
}

bool RuleResolver::broadcast(const NVar& exact) noexcept {
    bool all_succeeded = true; // do as much as possible, don't fast fail.
    for (Rule& eqn : pack)
        for (const NVar& var : eqn.vars)
            if (var.name == exact.name)
                if (!verify_then_remove(eqn.unknowns, var))
                    all_succeeded = false;
    return all_succeeded;
}

template <typename T>
std::optional<unsigned> RuleResolver::alg_consistent(const T& tmp) {
    unsigned step_count = 0;
    // save the current state, or the state might change while updating
    std::vector<std::pair<std::size_t, NVar>> to_be_updated;
    for (const auto& rule : pack) {
        if (rule.unknowns.size() + intersect_sets(rule.vars, tmp).size() == 1)
            to_be_updated.emplace_back(rule.unique_id, *rule.unknowns.cbegin());
    }
    for (const auto& rule : to_be_updated) {
        // recurse for the further variables
        if (!broadcast(rule.second)) {
            if (!verify_then_insert(cycle, rule.second))
                 return std::optional<unsigned>();
        }
    }
    for (const auto& rule : to_be_updated)
        order.add_alg(rule.first, rule.second);
    step_count += to_be_updated.size();
    return std::optional<unsigned>(step_count);
}

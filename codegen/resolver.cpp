/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This file implements routines that actually resolve the solving order.
 */

#include <utility>
#include <iterator>
#include <algorithm>
#include "iter_utils.hpp"
#include "set_support.hpp"
#include "rule_types.hpp"

template <class C>
std::vector<std::size_t> find_exact_m(std::vector<Rule>& pack, const C& vars) {
    std::vector<std::size_t> ret;
    for (auto& rule : pack)
        for (const auto& var : rule.vars)
            if (vars.count(var)) {
                ret.push_back(rule.unique_id);
                break;
            }
    return ret;
}

bool RuleResolver::validate_resolution() const noexcept {
    for (const Rule& eqn : pack)
        if (!eqn.unknowns.empty())
            return false;
    return true;
}

#ifndef	__cpp_deduction_guides
template <class T>
powerset<T> make_powerset(T b, T e, std::size_t sz) {
    return powerset<T>(b, e, sz);
}
#else
#define make_powerset powerset
#endif

unsigned RuleResolver::process(const CSF_set<NVar>& starts) {
    for (auto new_starts : make_powerset(inits.begin(), inits.end(), pack.size())) {
        std::vector<std::size_t> occurrences = find_exact_m(pack, inits);
        if (occurrences.size() == new_starts.size()) {
            unsigned step_count = 0;
            if (new_starts.size() == 1)
                order.add_step(occurrences[0], *inits.begin());
            else
                order.add_step(occurrences, new_starts);
            step_count += 1;
            auto Pack = pack;
            auto Starts = union_sets(starts, new_starts);
            auto Inits = inits;
            // to keep consistency of the system:
            // 1. the derivatives of the same variable must all be updated. (broadcast)
            // 2. the last variable in an algebraic equation must be computed
            //    once all the other variables are known.
            bool have_remaining_unknowns = false;
            for (const auto& var : Starts) {
                if (std::optional<unsigned> b_s = broadcast(var, Starts))
                    step_count += *b_s;
                else goto consistent_fail;
                if (std::optional<unsigned> a_s = alg_consistent())
                    step_count += *a_s;
                else goto consistent_fail;
consistent_fail:
                if (validate_resolution())
                    return step_count;
                else {
                    have_remaining_unknowns = true;
                    break;
                }
            }
            if (have_remaining_unknowns)
                try {
                    exclude(inits, new_starts);
                    step_count += process(Starts);
                }
                catch (RulePackCannotBeResolved) {
                    pack = std::move(Pack);
                    order.remove(step_count);
                    inits = std::move(Inits);
                    continue;
                }
            return step_count;
        }
    }
    throw RulePackCannotBeResolved();
}

std::optional<unsigned> RuleResolver::broadcast(const NVar& exact, const CSF_set<NVar>& except) {
    unsigned step_count = 0;
    bool all_succeeded = true;
    for (Rule& eqn : pack)
        for (const NVar& var : eqn.vars)
            if (var.name == exact.name && except.count(var) == 0)
                if (!verify_then_remove(eqn.unknowns, var)) // CAUTION: SIDE EFFECT
                    all_succeeded = false; // this way, we can postpone validation until the farthest caller
    const auto& all_forms = all_forms_indexed.at(exact.name);
    auto self_it = std::lower_bound(all_forms.begin(), all_forms.end(), exact.order);
    for (auto from = self_it, to = std::next(from); to != all_forms.end(); ++from, ++to) {
        order.add_step(NVar(exact.name, *from), *to);
        step_count += 1;
    }
    for (auto from = std::make_reverse_iterator(++self_it), to = std::next(from);
         to != std::make_reverse_iterator(all_forms.begin()); ++from, ++to) {
        order.add_step(NVar(exact.name, *from), *to);
        step_count += 1;
    }
    std::optional<unsigned> ret;
    if (all_succeeded)
        ret = step_count;
    return ret;
}

std::optional<unsigned> RuleResolver::alg_consistent() {
    unsigned step_count = 0;
    // save the current state, or the state might change while updating
    std::vector<Rule*> to_be_updated;
    for (auto& rule : pack) {
        if (rule.unknowns.size() == 1)
            to_be_updated.push_back(&rule);
    }
    for (auto ptr : to_be_updated) {
        Rule& eqn = *ptr;
        if (eqn.unknowns.empty()) {
            order.remove(step_count);
            return std::optional<unsigned>();
        }
        NVar to_update = *eqn.unknowns.begin(); // will be deleted soon, get a copy
        order.add_step(eqn.unique_id, to_update);
        step_count += 1;
        // recurse for the further variables
        if (auto further = broadcast(to_update))
            step_count += *further;
        else {
            order.remove(step_count);
            return std::optional<unsigned>();
        }
    }
    return std::optional<unsigned>(step_count);
}

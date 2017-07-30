/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This file implements routines that actually resolve the solving order.
 */

#include <utility>
#include <algorithm>
#include "iter_utils.hpp"
#include "set_support.hpp"
#include "rule_types.hpp"
#include "../common/typed_buffer.hpp"

struct eqn_try : iter_utils::non_trivial_end_iter<eqn_try> {
    typedef std::vector<Rule> eqn_container;
    typedef CSF_flat_set<NVar, NVar::Less> var_container;
    typedef CSF_flat_set<NVar, NVar::Less>::iterator var_iter;
    typedef combination<var_iter> comb_type;
    powerset<eqn_container> eqn_choice;
    var_container vars;
    placeholder<comb_type> selection;
    eqn_try(eqn_container& pack)
        : eqn_choice(pack, pack.size()) {
        for (const auto& eqn : *eqn_choice)
            vars.insert(eqn.vars.begin(), eqn.vars.end());
        new(&selection) comb_type(vars.begin(), vars.end(), eqn_choice->size());
    }
    eqn_try& operator++() {
        ++*selection;
        if (selection->exhausted()) {
            vars.clear();
            ++eqn_choice;
            for (const auto& eqn : *eqn_choice)
                vars.insert(eqn.vars.begin(), eqn.vars.end());
            selection->~comb_type();
            new(&selection) comb_type(vars.begin(), vars.end(), eqn_choice->size());
        }
        return *this;
    }
    std::pair<std::vector<std::size_t>, const std::vector<NVar>*> operator*() {
        std::vector<std::size_t> ret;
        ret.reserve(eqn_choice->size());
        for (std::size_t i = 0; i != eqn_choice->size(); ++i)
            ret.push_back((*eqn_choice)[i].unique_id);
        return std::make_pair(std::move(ret), &**selection);
    }
    bool exhausted() {
        return eqn_choice.exhausted();
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

unsigned RuleResolver::process(const CSF_set<NVar>& starts) {
    // TODO: Independent starts selection shall be deterministic, then we can
    // even eliminate optionals.
    for (auto attempt : eqn_try(pack)) {
        auto occurrences = attempt.first;
        const auto& new_starts = *attempt.second;
        unsigned step_count = 0;
        if (new_starts.size() == 1)
            order.add_alg(occurrences[0], *inits.begin());
        else
            order.add_alg_sys(occurrences, new_starts);
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
            if (!broadcast(var, Starts))
                goto consistent_fail;
            if (std::optional<unsigned> a_s = alg_consistent())
                step_count += *a_s;
            else goto consistent_fail;
consistent_fail:
            if (validate_resolution())
                return step_count;
            else {
                have_remaining_unknowns = true;
                goto end;
            }
        }
        if (have_remaining_unknowns)
            try {
                exclude(inits, new_starts);
                step_count += process(Starts);
            }
            catch (RulePackCannotBeResolved) {
                goto end;
            }
        return step_count;
end:
        pack = std::move(Pack);
        order.remove(step_count);
        inits = std::move(Inits);
    }
    throw RulePackCannotBeResolved();
}

bool RuleResolver::broadcast(const NVar& exact, const CSF_set<NVar>& except) {
    bool all_succeeded = true;
    for (Rule& eqn : pack)
        for (const NVar& var : eqn.vars)
            if (var.name == exact.name && except.count(var) == 0)
                if (!verify_then_remove(eqn.unknowns, var)) // CAUTION: SIDE EFFECT
                    all_succeeded = false; // do as much as possible, don't fast fail.
    return all_succeeded;
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
        order.add_alg(eqn.unique_id, to_update);
        step_count += 1;
        // recurse for the further variables
        if (!broadcast(to_update)) {
            order.remove(step_count);
            return std::optional<unsigned>();
        }
    }
    return std::optional<unsigned>(step_count);
}

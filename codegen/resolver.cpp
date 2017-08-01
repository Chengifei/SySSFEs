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
            ++eqn_choice;
            for (const auto& eqn : *eqn_choice)
                std::copy(eqn.vars.begin(), eqn.vars.end(), std::back_inserter(vars));
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
    ~eqn_try() {
        selection->~comb_type();
    }
};

bool RuleResolver::validate_resolution(const std::vector<NVar>& need_update) const noexcept {
    if (!is_subset(need_update, cycle))
        return false;
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
    for (auto attempt : make_eqn_try(pack)) {
        unsigned step_count = 0;
        const auto& starts = *attempt.second;
        {
        auto& occurrences = attempt.first;
        if (starts.size() == 1)
            order.add_alg(occurrences[0], *starts.begin());
        else if (starts.size() > 1)
            order.add_alg_sys(std::move(occurrences), starts);
        ++step_count;
        }
        auto Pack = pack;
        // to keep consistency of the system:
        // 1. the derivatives of the same variable must all be updated. (broadcast)
        // 2. the last variable in an algebraic equation must be computed
        //    once all the other variables are known.
        for (const auto& var : starts)
            if (!broadcast(var))
                goto fail;
        if (std::optional<unsigned> a_s = alg_consistent())
            step_count += *a_s;
fail:
        if (validate_resolution(starts))
            return true;
        else {
            pack = std::move(Pack);
            order.remove(step_count);
        }
    }
    return false;
}

bool RuleResolver::broadcast(const NVar& exact, bool enable_cycle) {
    bool all_succeeded = true; // do as much as possible, don't fast fail.
    for (Rule& eqn : pack)
        for (const NVar& var : eqn.vars)
            if (var.name == exact.name)
                if (!verify_then_remove(eqn.unknowns, var)
                    && (enable_cycle ? !verify_then_insert(cycle, var) : false))
                    all_succeeded = false;
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
        if (!broadcast(to_update, true)) {
            order.remove(step_count);
            return std::optional<unsigned>();
        }
    }
    return std::optional<unsigned>(step_count);
}

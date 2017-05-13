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

struct find_exact : iter_utils::non_trivial_end_iter<find_exact> {
    std::vector<Rule>::iterator it;
    std::vector<Rule>::iterator End;
    const NVar& var;
    find_exact (std::vector<Rule>& pack, const NVar& var)
        : it(pack.begin()), End(pack.end()), var(var) {
        while (!exhausted() && !it->vars.count(var))
            ++it;
    }
    find_exact& operator++() {
        do {
            ++it;
        } while (!exhausted() && !it->vars.count(var));
        return *this;
    }
    bool exhausted() {
        return it == End;
    }
    Rule& get() {
        return *it;
    }
};

template <class C>
std::vector<unsigned>
find_exact_m(std::vector<Rule>& pack, const C& vars) {
    std::vector<unsigned> ret;
    for (auto& rule : pack)
        if (is_subset(vars, rule.vars))
            ret.push_back(rule.unique_id);
    return ret;
}

unsigned RuleResolver::process(const CSF_flat_set<NVar, NVar::Less>& Requests,
                               const CSF_set<NVar>& IndieStarts) {
    for (auto variables : powerset(Requests.begin(), Requests.end())) {
        std::vector<unsigned> occurrences = find_exact_m(pack, variables);
        if (occurrences.size() == variables.size()) {
            unsigned step_count = 0;
            if (variables.size() == 1)
                order.add_step(occurrences[0], *variables.begin());
            else
                order.add_step(occurrences, variables);
            step_count += 1;
            std::vector<Rule> Pack = pack;
            auto requests = Requests;
            auto indieStarts = union_sets(IndieStarts, variables);
            // to keep consistency of the system:
            // 1. the derivatives of the same variable must all be updated. (broadcast)
            // 2. the last variable in an algebraic equation must be computed
            //    once all the other variables are known.
            try {
                for (auto var : variables) {
                    step_count += broadcast(var, requests, indieStarts);
                    step_count += alg_consistent(requests);
                }
            }
            catch (RulePackCannotBeResolved) {
                pack = std::move(Pack);
                order.remove(step_count);
                continue;
            }
            if (!requests.empty()) {
                try {
                    unsigned further_steps = process(requests, indieStarts);
                    step_count += further_steps;
                }
                catch (RulePackCannotBeResolved) {
                    pack = std::move(Pack);
                    order.remove(step_count);
                    continue;
                }
            }
            return step_count;
        }
    }
    throw RulePackCannotBeResolved();
}

unsigned RuleResolver::alg_consistent(CSF_flat_set<NVar, NVar::Less>& requests) {
    unsigned step_count = 0;
    auto Pack = pack;
    auto Requests = requests;
    // save the current state, or the state is changing while updating
    std::vector<Rule*> to_be_updated;
    for (auto& rule : pack) {
        if (rule.unknowns.size() == 1)
            to_be_updated.push_back(&rule);
    }
    for (auto ptr : to_be_updated) {
        Rule& eqn = *ptr;
        if (eqn.unknowns.empty()) {
            requests = std::move(Requests);
            pack = std::move(Pack);
            order.remove(step_count);
            throw RulePackCannotBeResolved();
        }
        const NVar& to_update = *eqn.unknowns.begin();
        order.add_step(eqn.unique_id, to_update);
        step_count += 1;
        // recurse for further variables
        try {
            unsigned further_steps = broadcast(to_update, requests);
            step_count += further_steps;
        }
        catch (RulePackCannotBeResolved) {
            requests = std::move(Requests);
            pack = std::move(Pack);
            order.remove(step_count);
            throw RulePackCannotBeResolved();
        }
    }
    return step_count;
}

unsigned RuleResolver::broadcast(const NVar& exact, CSF_flat_set<NVar, NVar::Less>& requests,
                            const CSF_set<NVar>& except) {
    unsigned step_count = 0;
    auto Pack = pack;
    auto Requests = requests;
    for (Rule& eqn : pack) {
        for (const NVar& var : eqn.vars) {
            if (var.name == exact.name && except.count(var) == 0) {
                if (verify_then_remove(eqn.unknowns, var)
                    && verify_then_remove(requests, var)) {
                    if (requests.empty())
                        goto exit_loops;
                }
                else {
                    requests = std::move(Requests);
                    pack = std::move(Pack);
                    order.remove(step_count);
                    throw RulePackCannotBeResolved();
                }
            }
        }
    }
exit_loops:
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
    return step_count;
}

unsigned RuleResolver::broadcast(const NVar& exact, CSF_flat_set<NVar, NVar::Less>& requests) {
    unsigned step_count = 0;
    auto Pack = pack;
    auto Requests = requests;
    for (Rule& eqn : pack) {
        for (const NVar& var : eqn.vars) {
            if (var.name == exact.name) {
                if (verify_then_remove(eqn.unknowns, var)
                    && verify_then_remove(requests, var)) {
                    if (requests.empty())
                        goto exit_loops;
                }
                else {
                    requests = std::move(Requests);
                    pack = std::move(Pack);
                    order.remove(step_count);
                    throw RulePackCannotBeResolved();
                }
            }
        }
    }
exit_loops:
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
    return step_count;
}

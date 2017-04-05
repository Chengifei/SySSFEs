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

std::ostream& operator<<(std::ostream& out, const NVar& v) {
    out << v.name;
    out << "(" << v.order << ")";
    return out;
}

class VarBroadcastFailure {};

unsigned RuleResolver::process(const CSF_flat_set<NVar, NVar::Less>& Requests, const CSF_set<NVar>& IndieStarts) {
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
            auto indieStarts = IndieStarts;
            update(indieStarts, variables);
            try {
                for (auto var : variables)
                    step_count += keep_consistent(var, requests, indieStarts);
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

CSF_flat_set<unsigned>
find_all(const std::vector<Rule>& pack, const NVar& var) {
    CSF_flat_set<unsigned> all_forms;
    for (auto& eqn : pack)
        for (auto& Var : eqn.vars)
            if (Var.name == var.name)
                all_forms.insert(Var.order);
    //all_forms.erase(var.order);
    return all_forms;
}

template <class _It>
unsigned link_derivatives(ResolvingOrder& order, _It begin, _It end, const NVar& var) {
    // THE FUNCTION ASSUMES VAR.ORDER TO BE IN THE LIST
    unsigned step_count = 0;
    auto self_it = std::lower_bound(begin, end, var.order);
    for (auto from = self_it, to = std::next(from); to != end; ++from, ++to) {
        order.add_step(NVar(var.name, *from), *to);
        step_count += 1;
    }
    for (auto from = std::make_reverse_iterator(++self_it), to = std::next(from);
         to != std::make_reverse_iterator(begin); ++from, ++to) {
        order.add_step(NVar(var.name, *from), *to);
        step_count += 1;
    }
    return step_count;
}

unsigned
RuleResolver::keep_consistent(NVar exact, CSF_flat_set<NVar, NVar::Less>& requests, CSF_set<NVar>& except) {
    auto all_forms = find_all(pack, exact);
    unsigned step_count = 0;
    auto Requests = requests;
    auto Pack = pack;
    auto Except = except;
    // to keep consistency of the system:
    // 1. the derivatives of the same variable must all be updated.
    // 2. the last variable in an algebraic equation must be computed
    //    once all the other variables are known.
    step_count += link_derivatives(order, all_forms.cbegin(), all_forms.cend(), exact);
    for (unsigned T : all_forms) {
        NVar to(exact.name, T);
        if (!verify_then_remove(requests, to)) { // to not found in requests
            if (!verify_then_remove(except, to)) {
                if (requests.empty() && except.empty())
                    continue;
                requests = Requests;
                pack = Pack;
                except = Except;
                order.remove(step_count);
                throw RulePackCannotBeResolved();
            }
        }
        if (except.count(to) == 0) // indepedent start node must not be used to solve algebraic equations.
            for (Rule& eqn : find_exact(pack, to)) {
                if (!verify_then_remove(eqn.unknowns, to)) { // attempting to update a variable twice
                    requests = Requests;
                    pack = Pack;
                    except = Except;
                    order.remove(step_count);
                    throw RulePackCannotBeResolved();
                }
                if (eqn.unknowns.size() == 1) {
                    NVar to_update = *eqn.unknowns.begin();
                    order.add_step(eqn.unique_id, to_update);
                    step_count += 1;
                    // recurse for further variables
                    try {
                        unsigned further_steps = keep_consistent(to_update, requests, except);
                        step_count += further_steps;
                    }
                    catch (RulePackCannotBeResolved) {
                        requests = Requests;
                        pack = Pack;
                        except = Except;
                        order.remove(step_count);
                        throw RulePackCannotBeResolved();
                    }
                }
            }
    }
    return step_count;
}

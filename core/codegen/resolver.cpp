/* Copyright 2017-2018 by Yifei Zheng
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file implements routines that actually resolve the solving order.
 */

#include <utility>
#include <algorithm>
#include <cstddef>
#include "resolver.hpp"
#include <boost/container/flat_set.hpp>

namespace std {
template<>
struct less<support::variable_designation> {
    bool operator()(const support::variable_designation& l, const support::variable_designation& r) const {
        return l.id < r.id || (l.id == r.id && l.order < r.order);
    }
};
}

namespace codegen {

resolved_sequence RuleResolver::get() {
    return std::move(sln);
}

bool RuleResolver::solve() {
    bool first = true;
    // I strongly belive this should be cached.
    std::size_t unresolved = std::count_if(pool.begin(), pool.end(),
                                           [](variable v) { return v.needs_update(); });
    while (unresolved) {
        switch (alg_consistent(first)) {
        case SUCCEEDED_AND_UPDATED:
            break;
        case SUCCEEDED_NONE_UPDATED:
            // It might be possible that we have fewer equations than variables
            // but still are able to solve the system. Here this is only for
            // programmatical safety that unresolved shall not exceed size
            // FIXME
            if (pack.size() < unresolved)
                return false;
            for (auto& attempt : powerset(pack.size(), unresolved)) {
                boost::container::flat_set<support::variable_designation> agg_unknowns;
                agg_unknowns.reserve(attempt.size());
                for (auto i : attempt) {
                    for (const auto& v : pack[i]) {
                        if (pool[v].needs_update()) {
                            agg_unknowns.insert(v);
                        }
                    }
                }
                if (agg_unknowns.size() == attempt.size()) {
                    for (const auto& var : agg_unknowns)
                        pool[var].update();
                    std::unique_ptr<std::size_t[]> rules = std::make_unique<std::size_t[]>(attempt.size());
                    std::size_t* end = rules.get() + attempt.size();
                    for (std::size_t i = 0; i != attempt.size(); ++i)
                        rules[i] = attempt[i];
                    std::unique_ptr<support::variable_designation[]> vars =
                        std::make_unique<support::variable_designation[]>(agg_unknowns.size());
                    std::copy(agg_unknowns.cbegin(), agg_unknowns.cend(), vars.get());
                    sln.push_back(step{std::move(rules), end, std::move(vars)});
                    break;
                }
            }
            break;
        case FAILED:
            return false;
        }
        first = false;
        std::size_t new_unresolved = std::count_if(pool.begin(), pool.end(),
                                   [](variable v) { return v.needs_update(); });
        if (new_unresolved != unresolved)
            unresolved = new_unresolved;
        else
            return false;
    }
    return true;
}

int RuleResolver::broadcast(const support::base_t& base) noexcept {
    bool updated = false;
    for (variable& var : pool.at(base))
        if (var.update())
            updated = true;
        else
            return FAILED;
    return updated ? SUCCEEDED_AND_UPDATED
                   : SUCCEEDED_NONE_UPDATED;
}

int RuleResolver::alg_consistent(bool use_start) {
    // save the current state, or the state might change while updating
    std::vector<std::pair<Rule*, support::variable_designation>> to_be_updated;
    for (auto& rule : pack) {
        if (rule.enabled) {
            const support::variable_designation* unknown = nullptr;
            for (const auto& var : rule) {
                const variable& state = pool[var];
                if (state.needs_update() && !(use_start && state.initialized)) {
                    if (unknown)
                        goto next_rule;
                    else
                        unknown = &var;
                }
            }
            to_be_updated.emplace_back(&rule, *unknown);
        }
    next_rule:;
    }

    if (to_be_updated.empty())
        return SUCCEEDED_NONE_UPDATED;

    for (const auto& update : to_be_updated) {
        int ret = broadcast(update.second.id);
        if (ret == FAILED)
             return FAILED;
    }

    for (const auto& rule : to_be_updated) {
        rule.first->enabled = false;
        std::unique_ptr<std::size_t[]> rules = std::make_unique<std::size_t[]>(1);
        std::unique_ptr<support::variable_designation[]> vars = std::make_unique<support::variable_designation[]>(1);
        rules[0] = rule.first - &*pack.begin();
        vars[0] = rule.second;
        std::size_t* end = rules.get() + 1;
        sln.push_back(step{ std::move(rules), end, std::move(vars) });
    }

    return SUCCEEDED_AND_UPDATED;
}

}

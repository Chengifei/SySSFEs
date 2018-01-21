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
#include <iter_utils.hpp>
#include "rule_types.hpp"

ResolvingOrder RuleResolver::get() {
    return std::move(order);
}

template <typename T>
struct VarStateDumper {
    T b, e;
    std::vector<char> update_states;
    std::vector<char> start_states;
    VarStateDumper(T b, T e): b(b), e(e) {
        std::size_t sz = std::distance(b, e);
        update_states.reserve(sz);
        start_states.reserve(sz);
        for (; b != e; ++b) {
            update_states.push_back((*b)->updated);
            start_states.push_back((*b)->as_start);
        }
    }
    void release() noexcept {
        b = e;
    }
    ~VarStateDumper() {
        for (std::size_t i = 0; b != e; ++i, ++b) {
            (*b)->updated = update_states[i];
            (*b)->as_start = start_states[i];
        }
    }
};

struct ResolvingOrderDumper {
    ResolvingOrder& o;
    std::size_t sz;
    ResolvingOrderDumper(ResolvingOrder& o): o(o), sz(o.size()) {}
    void release() noexcept {
        sz = o.size();
    }
    ~ResolvingOrderDumper() {
        o.resize(sz);
    }
};

struct start_selection : iter_utils::non_trivial_end_iter<start_selection> {
    typedef std::vector<Variable*>::iterator iterator;
    iterator _begin;
    const iterator _end;
    iterator cur = _begin;
    start_selection(iterator begin, iterator end) :
        _begin(begin), _end(end) {}
    void operator++(){
        if (cur == _end) {
            for (iterator it = _begin; it != cur; ++it)
                (*it)->as_start=false;
            ++_begin;
            (*_begin)->as_start = true;
        }
        else
            (*cur++)->as_start = true;
    }
    bool exhausted() const {
        return _begin == _end - 1 && cur == _end;
    }
    iter_utils::None operator*() const { return {}; }
};

bool RuleResolver::process() {
    // to keep the system consistent:
    // 1. the derivatives of the same variable must all be updated. (broadcast)
    // 2. the last variable in an algebraic equation must be computed
    //    once all the other variables are known.
    // Choose starts ...
    for (auto _ : start_selection(vars.begin(), vars.end())) {
        VarStateDumper<std::vector<Variable*>::iterator> _v(vars.begin(), vars.end());
        ResolvingOrderDumper _o(order);
        // not broadcasting because we select starts only for alg eqns.
        if (!alg_consistent())
            continue;
        // Check if all variables are solved
        if (std::all_of(vars.begin(), vars.end(),
            [](Variable* v) {return !v->need_update() || v->updated; })) {
            _v.release();
            _o.release();
            return true;
        }
    }
    return false;
}

int RuleResolver::broadcast(const Variable& exact, bool lenient_start) noexcept {
    bool updated = false;
    for (Variable* var : vars)
        if (var->name() == exact.name()) {
            if (var->updated && !lenient_start)
                return BROADCAST_FAILED;
            else {
                updated = true;
                var->updated = true;
            }
        }
    return updated ? BROADCAST_SUCCEED_AND_UPDATED : BROADCAST_SUCCEED_NONE_UPDATED;
}

bool RuleResolver::alg_consistent(bool update_start) {
    // save the current state, or the state might change while updating
    std::vector<std::pair<Rule*, Variable*>> to_be_updated;
    for (auto& rule : pack) {
        Variable* unknown = nullptr;
        for (auto var : rule) {
            if (var->need_update() && !var->updated && !var->as_start) {
                if (unknown)
                    goto next_rule;
                unknown = var;
            }
        }
        if (unknown)
            to_be_updated.emplace_back(&rule, unknown);
next_rule:;
    }
    bool recurse = false;
    for (const auto& update : to_be_updated) {
        // recurse for the further variables
        if (int ret = broadcast(*update.second, update_start); ret == BROADCAST_FAILED)
             return false;
        else if (ret == BROADCAST_SUCCEED_AND_UPDATED)
             recurse = true;
    }
    for (const auto& rule : to_be_updated)
        order.add_alg(rule.first, *rule.second);
    if (recurse)
        if (!alg_consistent(true))
            return false;
    return true;
}

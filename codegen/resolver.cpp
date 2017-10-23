/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This file implements routines that actually resolve the solving order.
 */

#include <utility>
#include <algorithm>
#include <cstddef>
#include "iter_utils.hpp"
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
    std::pair<iterator, iterator> operator*() const {
        return {_begin, cur};
    }
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
        // not broadcasting becuase we select starts only for alg eqns.
        if (!alg_consistent())
            goto fail;
        // Check if all variables are solved
        if (std::all_of(vars.begin(), vars.end(),
            [](Variable* v) {return !v->need_update() || v->updated; })) {
            _v.release();
            _o.release();
            return true;
        }
    fail:;
    }
    return false;
}

int RuleResolver::broadcast(const Variable& exact, bool lenient_start) noexcept {
    // do as much as possible, because when broadcasting overlaps with start,
    // it always fails but we're done in that case.
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
    if (recurse)
        if (!alg_consistent(true))
            return false;
    for (const auto& rule : to_be_updated)
        order.add_alg(rule.first, *rule.second);
    return true;
}

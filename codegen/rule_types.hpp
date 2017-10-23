/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This file defines a few data types to represent rules.
 */

#ifndef RULE_TYPES_HPP
#define RULE_TYPES_HPP
#include <string>
#include <cstddef>
#include <vector>
#include <array>
#include <optional>
#include "set_support.hpp"

class Variable {
protected: // Python-side may modify these fields, by inheriting
    std::string _name;
    std::array<std::uint8_t, 8> _order;
    // This field shall be std::vector<std::size_t>, but that'll be used so rarely.
    bool _can_start = false, _need_update = true;
public:
    bool updated = false;
    bool as_start = false;
    Variable(std::string name, std::uint8_t order)
        : _name(std::move(name)), _order{order} {}
    bool operator==(const Variable& rhs) const {
        return _name == rhs._name && _order == rhs._order;
    }
    const std::string& name() const noexcept {
        return _name;
    }
    const std::array<std::uint8_t, 8>& order() const noexcept {
        return _order;
    }
    bool can_start() {
        return _can_start;
    }
    bool need_update() {
        return _need_update;
    }
};
std::size_t hash_value(const Variable&);

using Rule = std::vector<Variable*>;

struct ResolvingOrder {
    class step_base {
        public:
            virtual bool type() const = 0;
            virtual ~step_base() {}
    };
    template <bool multiple> class step;
    std::vector<std::unique_ptr<step_base>> seq;
    ResolvingOrder() {}
    ResolvingOrder(ResolvingOrder&&) = default;
    ResolvingOrder& operator=(ResolvingOrder&&) = default;
    void add_alg(Rule*, Variable);
    void add_alg_sys(Rule*, Rule*, std::vector<Variable>);
    void resize(std::size_t sz) noexcept {
        seq.resize(sz);
    }
    void clear() noexcept {
        seq.clear();
    }
    std::size_t size() const noexcept {
        return seq.size();
    }
};

template <>
class ResolvingOrder::step<false> : public ResolvingOrder::step_base {
public:
    Rule* rule_id;
    Variable solve_for;
    step(Rule* id, Variable solve_for)
        : rule_id(id), solve_for(std::move(solve_for)) {}
    bool type() const override {
        return false;
    }
};

template <>
class ResolvingOrder::step<true> : public ResolvingOrder::step_base {
public:
    Rule* rule_begin_id, *rule_end_id;
    std::vector<Variable> solve_for;
    step(Rule* begin, Rule* end, std::vector<Variable> solve_for)
        : rule_begin_id(begin), rule_end_id(end), solve_for(std::move(solve_for)) {}
    bool type() const override {
        return false;
    }
};

class RuleResolver {
private:
    // std::vector<std::unique_ptr<Variable>> vars;
    std::vector<Variable*>& vars;
    std::vector<Rule>& pack;
    ResolvingOrder order;
public:
    RuleResolver(std::vector<Variable*>& vars, std::vector<Rule>& pack)
            noexcept : vars(vars), pack(pack) {}
    ResolvingOrder get();
    bool process();
private:
    /// The function may fail, and return false on fail.
    bool alg_consistent(bool update_start = false);
    enum : int { // return value enum
        BROADCAST_SUCCEED_AND_UPDATED,
        BROADCAST_SUCCEED_NONE_UPDATED,
        BROADCAST_FAILED
    };
    int broadcast(const Variable&, bool lenient_start = false) noexcept;
};
#endif

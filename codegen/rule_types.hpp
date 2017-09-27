/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This file defines a few data types to represent rules.
 */

#ifndef RULE_TYPES_HPP
#define RULE_TYPES_HPP
#include <string>
#include <cstdint>
#include <vector>
#include <array>
#include <optional>
#include "set_support.hpp"

enum class EqnSolver {
    ALG_S,
    ALG_M,
    DIFF_S
};

struct NVar {
    typedef std::string variable_type;
    variable_type name;
    std::array<std::uint8_t, 8> order;
    // This field shall be std::vector<std::size_t>, but that'll be used so rarely.
    NVar() {}
    NVar(std::string name, std::uint8_t order)
        : name(name), order{order} {}
    bool operator==(const NVar& rhs) const {
        return name == rhs.name && order == rhs.order;
    }
    struct Less {
        bool operator()(const NVar& self, const NVar& rhs) const {
            if (self.name == rhs.name)
                return self.order > rhs.order;
            else
                return self.name < rhs.name;
        }
    };
};
std::size_t hash_value(const NVar&);

struct Rule {
    std::size_t unique_id;
    CSF_set<NVar> vars;
    CSF_set<NVar> unknowns;
    Rule(){}
    Rule(std::size_t id, CSF_set<NVar> vars)
        : unique_id(id), vars(std::move(vars)), unknowns(this->vars) {}
};

struct ResolvingOrder {
    class step_base {
        public:
            virtual EqnSolver type() const = 0;
            // HACK: we can't return reference here...
            virtual const std::vector<NVar> updates() const = 0;
            virtual ~step_base() {}
    };
    template <EqnSolver type> class step;
    std::vector<std::unique_ptr<step_base>> seq;
    ResolvingOrder() {}
    ResolvingOrder(ResolvingOrder&&) = default;
    ResolvingOrder& operator=(ResolvingOrder&&) = default;
    void add_alg(std::size_t, NVar);
    void add_alg_sys(std::vector<std::size_t>, std::vector<NVar>);
    void remove(std::size_t idx) noexcept {
        seq.resize(seq.size() - idx);
    }
    void clear() noexcept {
        seq.clear();
    }
    std::size_t size() const noexcept {
        return seq.size();
    }
};

template <>
class ResolvingOrder::step<EqnSolver::ALG_S> : public ResolvingOrder::step_base {
public:
    std::size_t rule_id;
    NVar solve_for;
    step(std::size_t id, NVar solve_for)
        : rule_id(id), solve_for(std::move(solve_for)) {}
    const std::vector<NVar> updates() const override {
        return std::vector<NVar>{solve_for};
    }
    EqnSolver type() const override {
        return EqnSolver::ALG_S;
    }
};

template <>
class ResolvingOrder::step<EqnSolver::ALG_M> : public ResolvingOrder::step_base {
public:
    std::vector<std::size_t> rules;
    std::vector<NVar> solve_for;
    step(std::vector<std::size_t> rules, std::vector<NVar> solve_for)
        : rules(std::move(rules)), solve_for(std::move(solve_for)) {}
    const std::vector<NVar> updates() const override {
        return solve_for;
    }
    EqnSolver type() const override {
        return EqnSolver::ALG_M;
    }
};

struct RuleResolver {
private:
    std::vector<Rule> pack;
    const CSF_set<NVar>& knowns;
    CSF_set<NVar> cycle;
    ResolvingOrder order;
public:
    RuleResolver(std::vector<Rule> pack, const CSF_set<NVar>& knowns)
        : pack(std::move(pack)), knowns(knowns) {}
    ResolvingOrder get();
    bool process();
private:
    /// The function may fail, and returns empty optionals on fail.
    /// use tmp to address variables that are marked solved but shall not be considered in alg_consistent
    /// e.g. starting equation since they are unconditionally solved but require cycle update.
    template <typename T>
    std::optional<unsigned> alg_consistent(const T& tmp);
    /// The function may fail, and returns false on fail.
    bool broadcast(const NVar&) noexcept;
    bool validate_resolution() const noexcept;
};
#endif

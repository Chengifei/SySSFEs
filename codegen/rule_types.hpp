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
#include "set_support.hpp"
#include <unordered_map>

enum class Type {
    SCALAR, VECTOR
};

enum class EqnSolver {
    ALG_S,
    ALG_M,
    DIFF_S,
};

struct NVar {
    std::string name;
    unsigned order;
    Type type;
    NVar() {}
    NVar(std::string name, unsigned order) : name(name), order(order) {}
    bool operator==(const NVar& rhs) const {
        return name == rhs.name && order == rhs.order;
    }
    struct Less {
        bool operator()(const NVar& self, const NVar& rhs) const {
            if (self.name == rhs.name)
                return self.order < rhs.order;
            else 
                return self.name < rhs.name;
        }
    };
};
std::size_t hash_value(const NVar&);

struct Rule {
    unsigned unique_id;
    CSF_set<NVar> vars;
    CSF_set<NVar> unknowns;
    Rule(){}
    Rule(unsigned id, CSF_set<NVar> vars)
        : unique_id(id), vars(vars), unknowns(vars) {}
};

struct ResolvingOrder {
    class step_base {
        public:
            virtual unsigned type() const = 0;
            //virtual std::vector<NVar> get_source() const = 0;
            virtual ~step_base() {}
    };
    template <EqnSolver type> class step;
    std::vector<std::unique_ptr<step_base>> seq;
    ResolvingOrder() {}
    void add_step(unsigned, NVar);
    void add_step(std::vector<unsigned>, std::vector<NVar>);
    void add_step(NVar, unsigned);
    void remove(unsigned idx) noexcept {
        seq.resize(seq.size() - idx);
    }
    std::size_t size() const {
        return seq.size();
    }
};

template <>
class ResolvingOrder::step<EqnSolver::ALG_S> : public ResolvingOrder::step_base {
public:
    unsigned rule_id;
    NVar solve_for;
    step(unsigned id, NVar solve_for) : rule_id(id), solve_for(solve_for) {}
    unsigned type() const override {
        return static_cast<unsigned>(EqnSolver::ALG_S);
    }
};

template <>
class ResolvingOrder::step<EqnSolver::ALG_M> : public ResolvingOrder::step_base {
public:
    std::vector<unsigned> rules;
    std::vector<NVar> solve_for;
    step(std::vector<unsigned> rules, std::vector<NVar> solve_for) : rules(rules), solve_for(solve_for) {}
    unsigned type() const override {
        return static_cast<unsigned>(EqnSolver::ALG_M);
    }
};

template <>
class ResolvingOrder::step<EqnSolver::DIFF_S> : public ResolvingOrder::step_base {
public:
    NVar from, to;
    step(NVar from, NVar to) : from(from), to(to) {}
    unsigned type() const override {
        return static_cast<unsigned>(EqnSolver::DIFF_S);
    }
};

class RulePackCannotBeResolved {};

struct RuleResolver {
private:
    std::vector<Rule> pack;
    ResolvingOrder& order;
    const std::unordered_map<std::string, CSF_flat_set<unsigned>> all_forms_indexed;
public:
    RuleResolver(std::vector<Rule> pack, ResolvingOrder& order, decltype(all_forms_indexed) all_forms_indexed)
        : pack(std::move(pack)), order(order), all_forms_indexed(std::move(all_forms_indexed)) {}
    unsigned process(const CSF_flat_set<NVar, NVar::Less>& Requests, const CSF_set<NVar>& IndieStarts);
private:
    // all functions guarantee strong exception safety
    unsigned alg_consistent(CSF_flat_set<NVar, NVar::Less>& requests);
    unsigned broadcast(const NVar&, CSF_flat_set<NVar, NVar::Less>& requests, const CSF_set<NVar>& except);
    unsigned broadcast(const NVar&, CSF_flat_set<NVar, NVar::Less>& requests);
};
#endif

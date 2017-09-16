/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This file defines functions that are used to convert data from and to Python.
 */

#include "../common/python_common.hpp"
#include "rule_types.hpp"
#include "cNVar.hpp"
#include <vector>

PyObject* write_step(ResolvingOrder::step_base* step) {
    if (step->type() == EqnSolver::ALG_S) {
        std::size_t rule_id = static_cast<ResolvingOrder::step<EqnSolver::ALG_S>*>(step)->rule_id;
        const NVar& solve_for = static_cast<ResolvingOrder::step<EqnSolver::ALG_S>*>(step)->solve_for;
        auto tuple = PyTuple_New(2);
        PyOnly(PyTuple_SetItem(tuple, 0, PyLong_FromLong(rule_id)), 0);
        PyOnly(PyTuple_SetItem(tuple, 1, make_NVar(solve_for)), 0);
        return tuple;
    }
    else if (step->type() == EqnSolver::ALG_M) {
        const auto& rule_id = static_cast<ResolvingOrder::step<EqnSolver::ALG_M>*>(step)->rules;
        const auto& solve_for = static_cast<ResolvingOrder::step<EqnSolver::ALG_M>*>(step)->solve_for;
        auto tuple = PyTuple_New(2);
        auto rules = PyList_New(0);
        auto solves = PyList_New(0);
        for (std::size_t i : rule_id)
            PyList_Append(rules, PyLong_FromLong(i));
        for (const NVar& i : solve_for)
            PyList_Append(rules, make_NVar(i));
        PyOnly(PyTuple_SetItem(tuple, 0, rules), 0);
        PyOnly(PyTuple_SetItem(tuple, 1, solves), 0);
        return tuple;
    }
    else
        throw -1;
}

PyObject* tuplize_order(const ResolvingOrder& order) {
    try {
        auto ret = PyExc(PyList_New(0), nullptr);
        for (auto& step : order.seq)
            PyList_Append(ret, write_step(step.get()));
        return ret;
    }
    catch (Python_API_Exception) {
        PyErr_SetString(PyExc_RuntimeError, "Unknown error occurred in tuplize_order which shouldn't throw");
        return nullptr;
    }
}

PyObject* resolve(PyObject*, PyObject* args) {
    std::vector<Rule> Pack;
    CSF_set<NVar> knowns;
    try {
        PyObject* pack;
        PyObject* known;
        PyOnly(PyArg_ParseTuple(args, "OO", &pack, &known), true);
        std::size_t sz = PyExc(PySequence_Size(pack), -1);
        Pack.reserve(sz);
        for (std::size_t i = 0; i != sz; ++i) {
            auto r = PyList_GET_ITEM(pack, i);
            PyScoped diffs(PyExc(PyObject_GetAttrString(r, "diffs"), nullptr));
            PyScoped iter(PyExc(PyObject_GetIter(diffs), nullptr));
            PyScoped item(PyIter_Next(iter));
            CSF_set<NVar> vars;
            for (; item; item = PyScoped(PyIter_Next(iter))) {
                const cNVar* i = static_cast<cNVar*>(item.get());
                vars.insert(i->var);
            }
            Pack.emplace_back(i, std::move(vars));
        }
        PyScoped iter(PyExc(PyObject_GetIter(known), nullptr));
        PyScoped item(PyIter_Next(iter));
        for (; item; item = PyScoped(PyIter_Next(iter))) {
            knowns.insert(static_cast<cNVar*>(item.get())->var);
        }
    }
    catch (Python_API_Exception) {
        PyErr_SetString(PyExc_RuntimeError, "Error converting python objects to C++ equivalent");
        return nullptr;
    }
    RuleResolver Resolver(std::move(Pack), knowns);
    if (!Resolver.process()) {
        PyErr_SetString(PyExc_ValueError, "Rule pack cannot be resolved");
        return nullptr;
    }
    return tuplize_order(Resolver.get());
}

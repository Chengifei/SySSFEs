/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This file defines functions that are used to convert data from and to Python.
 */

#include "../common/python_common.hpp"
#include "rule_types.hpp"
#include "cNVar.hpp"
#include "cRule.hpp"

PyObject* write_step(const Rule* const base, ResolvingOrder::step_base* step) {
    if (!step->type()) {
        std::size_t rule_id = static_cast<ResolvingOrder::step<false>*>(step)->rule_id - base;
        const Variable& solve_for = static_cast<ResolvingOrder::step<false>*>(step)->solve_for;
        auto tuple = PyTuple_New(2);
        PyOnly(PyTuple_SetItem(tuple, 0, PyLong_FromLong(rule_id)), 0);
        PyOnly(PyTuple_SetItem(tuple, 1, make_NVar(solve_for)), 0);
        return tuple;
    }
    else {
        std::size_t rule_begin_id = static_cast<ResolvingOrder::step<true>*>(step)->rule_begin_id - base;
        std::size_t rule_end_id = static_cast<ResolvingOrder::step<true>*>(step)->rule_end_id - base;
        const auto& solve_for = static_cast<ResolvingOrder::step<true>*>(step)->solve_for;
        auto tuple = PyTuple_New(2);
        auto rules = PyTuple_New(2);
        auto solves = PyList_New(0);
        PyOnly(PyTuple_SetItem(rules, 0, PyLong_FromLong(rule_begin_id)), 0);
        PyOnly(PyTuple_SetItem(rules, 1, PyLong_FromLong(rule_end_id)), 0);
        for (const Variable& i : solve_for)
            PyList_Append(rules, make_NVar(i));
        PyOnly(PyTuple_SetItem(tuple, 0, rules), 0);
        PyOnly(PyTuple_SetItem(tuple, 1, solves), 0);
        return tuple;
    }
}

PyObject* tuplize_order(const Rule* const base, const ResolvingOrder& order) {
    try {
        auto ret = PyExc(PyList_New(0), nullptr);
        for (auto& step : order.seq)
            PyList_Append(ret, write_step(base, step.get()));
        return ret;
    }
    catch (Python_API_Exception) {
        PyErr_SetString(PyExc_RuntimeError, "Unknown error occurred in tuplize_order which shouldn't throw");
        return nullptr;
    }
}

PyObject* resolve(PyObject*, PyObject* args) {
    std::vector<Variable*> vars;
    std::vector<Rule> eqns;
    try {
        PyObject* pack, *der_gen;
        PyOnly(PyArg_ParseTuple(args, "O!O", &PyList_Type, &pack, &der_gen), 1);
        // steal resource from Python
        std::size_t sz = PyList_GET_SIZE(pack);
        for (std::size_t i = 0; i != sz; ++i) {
            PyObject* r = PyList_GET_ITEM(pack, i);
            cRule& rule = *static_cast<cRule*>(r);
            PyScoped Ldiffs(PyExc(PyObject_CallFunctionObjArgs(der_gen, rule.lhs,
                                                               nullptr), nullptr));
            PyScoped Rdiffs(PyExc(PyObject_CallFunctionObjArgs(der_gen, rule.rhs,
                                                               nullptr), nullptr));
            std::vector<Variable*> eqn;
            for (PyScoped item(PyIter_Next(Ldiffs)); item.get(); item = PyScoped(
                    PyIter_Next(Ldiffs))) {
                Py_INCREF(item.get());
                eqn.push_back(static_cast<cNVar*>(item.get()));
                vars.push_back(static_cast<cNVar*>(item.get()));
            }
            for (PyScoped item(PyIter_Next(Rdiffs)); item.get(); item = PyScoped(
                    PyIter_Next(Rdiffs))) {
                Py_INCREF(item.get());
                eqn.push_back(static_cast<cNVar*>(item.get()));
                vars.push_back(static_cast<cNVar*>(item.get()));
            }
            PyOnly(PyErr_Occurred(), nullptr);
            eqns.push_back(std::move(eqn));
        }
    }
    catch (Python_API_Exception) {
        return nullptr;
    }
    RuleResolver Resolver(vars, eqns);
    // unique vars somehow
    auto base = &*eqns.begin();
    if (!Resolver.process()) {
        PyErr_SetString(PyExc_ValueError, "Rule pack cannot be resolved");
        PyTraceback(__FUNCTION__, __FILE__, __LINE__ - 2);
        return nullptr;
    }
    for (Variable* var : vars)
        Py_DECREF(static_cast<cNVar*>(var));
    return tuplize_order(base, Resolver.get());
}

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
        PyObject* pack, *pyvars;
        PyOnly(PyArg_ParseTuple(args, "O!O!", &PyList_Type, &pack, &PyList_Type, &pyvars), 1);
        std::size_t var_sz = PyList_GET_SIZE(pyvars);
        vars.reserve(var_sz);
        // steal resource from Python
        for (std::size_t i = 0; i != var_sz; ++i) {
            Py_INCREF(PyList_GET_ITEM(pyvars, i));
            vars.push_back(static_cast<cNVar *>(PyList_GET_ITEM(pyvars, i)));
        }
        for (std::size_t i = 0, sz = PyList_GET_SIZE(pack); i != sz; ++i) {
            PyObject* pyeqn = PyList_GET_ITEM(pack, i);
            std::size_t eqn_sz = PyList_GET_SIZE(pyeqn);
            Rule eqn;
            eqn.reserve(eqn_sz);
            for (std::size_t i = 0; i != eqn_sz; ++i)
                eqn.push_back(vars[PyLong_AsSize_t(PyList_GET_ITEM(pyeqn, i))]);
            eqns.push_back(std::move(eqn));
        }
    }
    catch (Python_API_Exception) {
        return nullptr;
    }
    RuleResolver Resolver(vars, eqns);
    // FIXME: unique vars somehow
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

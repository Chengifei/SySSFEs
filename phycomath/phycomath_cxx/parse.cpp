/*
This is the string and string-represented math expression processing module of Phyco.
Copyright 2016 by Yifei Zheng
All rights reserved.
*/
#if defined _MSC_VER && defined PY_EXT
#define BOOST_PYTHON_STATIC_LIB
#endif
//#include <boost/pool/pool_alloc.hpp>
#ifdef PHYCO_DEBUG
#include <cassert>
#include <typeinfo>
#ifndef _MSC_VER
#include <cxxabi.h>
#endif
#endif
#ifndef PY_EXT
#include <iostream>
#include <chrono>
#else
#include <memory>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#endif
#include "phycomath.hpp"

auto init() {
    math::operators::init();
    auto keywords = math::operators::infixlist;
    keywords.insert(math::operators::bilist.begin(),
                    math::operators::bilist.end());
    return keywords;
}


template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T*>& v) {
    for (T* i : v)
        os << *i << ", ";
    os << "\b\b  \b";
    return os;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    os << "[";
    for (auto i : v)
        os << i << ", ";
    os << "\b\b] \b";
    return os;
}

template <template<class, class> class T, class U, class V>
std::ostream& operator<<(std::ostream& os, const T<U, V>& v) {
    os << "[";
    for (auto i : v)
        os << i.first << ", ";
    os << "\b\b] \b";
    return os;
}

template <typename T>
T remove_at(std::vector<T>&v, typename std::vector<T>::size_type n)
{
    T ans = std::move_if_noexcept(v[n]);
    v[n] = std::move_if_noexcept(v.back());
    v.pop_back();
    return ans;
}

#ifdef PY_EXT

namespace py = boost::python;

struct PyObject_conv_visitor :public utils::variant::static_visitor<PyObject*> {
    static result_type convert(const math::arg_t& v) noexcept {
        return utils::variant::apply_visitor(PyObject_conv_visitor(), v);
    }

    template <class T>
    PyObject* operator()(const T& arg) const {
        return py::incref(py::object(arg).ptr());
    }
};

namespace for_python {
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(exec_fp, math::expression_tree::fexec, 0, 1);
auto parse(const string& str) noexcept {
    static trie Trie(::init());
    return std::make_shared<math::expression_tree>(math::parse(str, Trie));
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(fNsolve_fp, math::eqn_t::fNsolve, 0, 2);

auto make_equation(std::shared_ptr<math::expression_tree> lhs,
                          std::shared_ptr<math::expression_tree> rhs) noexcept {
    return std::make_shared<math::eqn_t>(*lhs, *rhs);
}

}

BOOST_PYTHON_MODULE(phycomath)
{
    using namespace for_python;
    py::to_python_converter<math::arg_t, PyObject_conv_visitor>();

    py::class_<math::expression_tree, boost::noncopyable, std::shared_ptr<math::expression_tree>>("expr_t", py::no_init)
        .def_readonly("mroot", &math::expression_tree::mroot)
        .def("exec", &math::expression_tree::fexec,exec_fp())
        .def("__repr__", &math::expression_tree::repr);
    //py::class_<math::expression_tree::node, boost::noncopyable>("expr_node",py::no_init);

    py::class_<math::eqn_t, boost::noncopyable, std::shared_ptr<math::eqn_t>>("equation", py::no_init)
        .def("solve",&math::eqn_t::fNsolve,fNsolve_fp());

    //py::def("parse", process);

    py::def("parse", parse);
    //py::def("exec", exec);
    //py::def("exec", execd);
    py::def("makeEquation", make_equation);
    //py::def("simplify", math::simplify::simplify);
    //py::def("solve", math::solver::nnsolve);

}

#else
int main() {
    std::cin.exceptions(std::istream::failbit|std::istream::badbit);
    trie Trie(::init());
    string str;
    std::vector<math::expression_tree::node*> vec;
    while (true) {
        math::expression_tree expr1;
        std::getline(std::cin, str);
        expr1 = math::parse(str, Trie);
        std::cout<< expr1.repr() << " = ";
        std::cout<< expr1.fexec() << "\n";
    }
}
#endif

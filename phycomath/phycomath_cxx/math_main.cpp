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
#endif
#include "phycomath.hpp"

auto init() {
    math::operators::init();
    auto keywords = math::operators::infixlist;
    keywords.insert(math::operators::bilist.begin(),
                    math::operators::bilist.end());
    return keywords;
}

#ifdef PY_EXT

namespace py = boost::python;

namespace for_python {

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(exec_fp, math::expression_tree::fexec, 0, 1);
inline auto parse(string str) noexcept {
    static auto d = init();
    return std::make_shared<math::expression_tree>(math::parse(std::move(str)));
}

auto make_equation(std::shared_ptr<math::expression_tree>& a, std::shared_ptr<math::expression_tree>& b) noexcept {
    auto temp = std::make_shared<math::eqn_t>(std::move(*a), std::move(*b));
    a.reset();
    b.reset();
    return temp;
}

double (math::expression_tree::*expression_tree_fexec_wrap)(const py::dict&) const = &math::expression_tree::fexec;
}

BOOST_PYTHON_MODULE(math_cxx)
{
    using namespace for_python;

    py::class_<math::expression_tree, boost::noncopyable, std::shared_ptr<math::expression_tree>>("expr_t", py::no_init)
        .def("exec", expression_tree_fexec_wrap)
        .def("__repr__", &math::expression_tree::repr);

    py::class_<math::eqn_t, boost::noncopyable, std::shared_ptr<math::eqn_t>>("equation", py::no_init)
        .def("solve", &math::eqn_t::fNsolve);

    py::def("parse", parse);
    //py::def("exec", exec);
    //py::def("exec", execd);
    py::def("makeEquation", make_equation);
    py::def("simplify", math::simplify::simplify);
    //py::def("solve", math::solver::nnsolve);

}

#else
int main() {
    string str;
    while (true) {
        math::expression_tree expr1;
        std::cin >> str;
        if (std::cin.eof())
            return 0;
        expr1 = math::parse(str);
        std::cout<< expr1.repr() << " = ";
        std::cout<< expr1.fexec() << "\n";
    }
}
#endif

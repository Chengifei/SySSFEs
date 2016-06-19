/*
This is the string and string-represented math expression processing module of Phyco.
Copyright 2016 by Yifei Zheng
All rights reserved.
*/
#define PY_EXT
#if defined __GNUC__ && defined PY_EXT
#warning Python extension interface are not fully supported and tested\
There are known issue that the program cannot be linked on Windows(as tested on Win7 x64)
#warning If anyone here succeeded to link it to a usable pyd, please notify the author.
#warning It seems that it's because Python from executable installer are compiled \
with MSVC compiler, and MinGW compiler wouldn't have access to its dynamic-library.
#endif
#if defined _MSC_VER && defined PY_EXT
#define BOOST_PYTHON_STATIC_LIB
#endif
#include "phycomath.h"
#ifdef PHYCO_DEBUG
#include <cassert>
#include <typeinfo>
#ifndef _MSC_VER
#include <cxxabi.h>
#endif
#endif
#ifndef PY_EXT
#include <iostream>
#else
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#endif

void init() {
    math::operators::init();
    auto keywords = math::operators::infixlist;
    keywords.insert(math::operators::bilist.begin(),
                    math::operators::bilist.end());
    trie::build(keywords);
    trie::reset();
}

#ifdef PY_EXT
BOOST_PYTHON_MODULE(phycomath)
{
    namespace py = boost::python;
    py::class_<std::vector<string> >("std::vector<string>", py::no_init)
        .def(py::vector_indexing_suite<std::vector<string> >())
        .def("__repr__", &strvec_repr);

    py::class_<math::arg_t>("arg", py::init<double>())
        .def(py::init<int>())
        .def("__repr__", &math::convert::Arg2str);

    py::class_<math::Op>("Op", py::no_init)
        .def("__repr__", &math::Op::repr);

    py::class_<math::plexpr>("plexpr", py::init<math::Op, math::arg_t, math::arg_t>())
        .def("exec", &math::plexpr::exec)
        .def("__repr__", &math::convert::expr2str);

    py::def("parse", process);

    py::def("mparse", math::parse);

    void(*initpy)(void) = &init;
    py::def("init", initpy);

}
#else
int main(int argc, char* argv[]) {
    ::init();
    auto _ = new math::plexpr(math::parse(argv[1]));
    //std::cout << math::convert::expr2str(*_) << std::endl;
    return 0;
}
#endif
/*
Copyright 2016 by Yifei Zheng
This File defines the interface of geom_support module, accessed from python.
It consists of a series of unlisted boost.python API to retrieve exposed vector.
These are internal API of boost, and is not documented. COMPATIBILITY SHOULD BE
CAREFULLY TESTED AGAINST ANY FURTHER BOOST RELEASE.
*/

#if defined _MSC_VER && defined PY_EXT
#define BOOST_PYTHON_STATIC_LIB
#endif
#include <boost/python.hpp>
#include "geom_support.hpp"
#include <typeinfo>

using namespace boost::python;

struct vector_from_python {
    vector_from_python() {
        converter::registry::push_back(
            &convertible,
            &construct,
            boost::python::type_id<vector<double>>());
    }
    static void* convertible(PyObject* ptr) {
        return ptr;
    }
    static void construct(PyObject* ptr, converter::rvalue_from_python_stage1_data* data) {
        objects::instance<>* self = reinterpret_cast<objects::instance<>*>(ptr);
        void* found = nullptr;
        for (instance_holder* match = self->objects; match != 0; match = match->next()) {
            found = match->holds(typeid(vector<double>), false);
            if (found)
                break;
        }
        data->convertible = found;
    }
};


BOOST_PYTHON_MODULE(geom_support) {
    vector_from_python();
    py::def("isCoplannar", is_coplannar<double>);
    py::def("distBetweenLines", distance_between_lines<double>);
    py::def("checkProjectionIntersect", check_projection_intersect<double>);
    py::def("isOnLine", is_on_line<double>);
}

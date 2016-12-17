#ifdef _MSC_VER
#define BOOST_PYTHON_STATIC_LIB
#endif
#include <boost/python.hpp>
#include "vector.hpp"

BOOST_PYTHON_MODULE(vector) {
    py::class_<vector<double>>("vector", py::init<py::list>())
        .def(py::init<py::list, int>())
        .def(py::init<double, double, double>())
        .def(py::self + py::self)
        .def(-py::self)
        .def(py::self - py::self)
        .def(py::self / double())
        .def(py::self == py::self)
        .def("dot", &vector<double>::inner)
        .def("mul", &vector<double>::mul)
        .def("cross", &vector<double>::cross)
        .def("__abs__", &vector<double>::mag)
        .def("angle", &vector<double>::angle)
        .def("unit", &vector<double>::unit)
        .def("proj", &vector<double>::proj)
        .def("__repr__", &vector<double>::str)
        .def("__str__", &vector<double>::str);
}

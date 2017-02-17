#include <boost/python.hpp>
#include <Python.h>

namespace py = boost::python;

PyObject* dtoBinary(double d) {
    return PyBytes_FromStringAndSize((const char*)&d, sizeof(double));
}

PyObject* itoBinary(unsigned i) {
    return PyBytes_FromStringAndSize((const char*)&i, sizeof(unsigned));
}

PyObject* ctoBinary(char c) {
    return PyBytes_FromStringAndSize((const char*)&c, sizeof(char));
}

BOOST_PYTHON_MODULE(toBinary) {
    py::def("double", dtoBinary);
    py::def("integer", itoBinary);
    py::def("char", ctoBinary);
}

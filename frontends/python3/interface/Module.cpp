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
 */

#include <new>
#include <LLVM/driver.hpp>
#include "Module.hpp"
#include "cTypes.hpp"
#include <string>

support::Expr parse(std::string, std::size_t&, std::size_t&);

int module_init(PyObject* self, PyObject* args, PyObject*) {
    new(static_cast<cModule*>(self)) cModule;
    PyObject* ostream;
    if (!PyArg_ParseTuple(args, "O", &ostream))
        return -1;
    static_cast<cModule*>(self)->output = PyObject_GetAttrString(ostream, "write");
    Py_INCREF(static_cast<cModule*>(self)->output);
    return 0;
}

PyObject* module_add_rule(PyObject* self, PyObject* args) {
    std::size_t max_comb, max_order;
    unsigned line;
    unsigned col;
    const char* str;
    if (!PyArg_ParseTuple(args, "(II)s", &line, &col, &str))
        return nullptr;
    static_cast<cModule*>(self)->rules.push_back((parse(str, max_order, max_comb)));
    return PyTuple_Pack(2, PyLong_FromLong(max_comb), PyLong_FromLong(max_order));
}

PyObject* module_ctrl(PyObject* self, PyObject* args) {
    PyObject* pname = PyTuple_GET_ITEM(args, 0);
    auto vn = PyUnicode_AsUTF8(pname);
    double start = PyFloat_AS_DOUBLE(PyScoped(PyFloat_FromString(PyTuple_GET_ITEM(args, 1))).get());
    double step = PyFloat_AS_DOUBLE(PyScoped(PyFloat_FromString(PyTuple_GET_ITEM(args, 2))).get());
    double end = PyFloat_AS_DOUBLE(PyScoped(PyFloat_FromString(PyTuple_GET_ITEM(args, 3))).get());
    static_cast<cModule*>(self)->add_ctrl(vn, start, step, end);
    Py_RETURN_NONE;
}

PyObject* module_add_field(PyObject* self, PyObject* args) {
    PyObject* tp;
    const char* name;
    if (!PyArg_ParseTuple(args, "Os", &tp, &name))
        return nullptr;
    static_cast<cModule*>(self)->object_type.add(name, static_cast<cTypes*>(tp)->type);
    Py_RETURN_NONE;
}

PyObject* module_obj(PyObject* self, PyObject* args) {
    auto str = PyUnicode_AsUTF8(PyTuple_GET_ITEM(args, 0));
    static_cast<cModule*>(self)->add_obj(str, (void*)nullptr);
    Py_RETURN_NONE;
}

PyObject* warn(PyObject* self, PyObject* msg) {
    auto& cself = *static_cast<cModule*>(self);
    const char* a = PyUnicode_AsUTF8(msg);
    cself.diag.issue(DiagCtl::LEVEL::WARNING, "%s", a);
    auto ret = PyObject_Call(cself.output,
        PyTuple_Pack(1, PyUnicode_FromString(cself.str_out.str().c_str())), nullptr);
    cself.str_out.str("");
    cself.str_out.clear();
    return ret;
}

static PyMethodDef Methods[] = {
    {"add_rule", module_add_rule, METH_VARARGS, ""},
    {"add_field", module_add_field, METH_VARARGS, ""},
    {"add_ctrl", module_ctrl, METH_VARARGS, ""},
    {"add_obj", module_obj, METH_VARARGS, ""},
    {"warn", warn, METH_O, ""},
    {nullptr}
};

PyTypeObject cModule_type {
        PyVarObject_HEAD_INIT(NULL, 0)
        "llvm.Module",          /* tp_name */
        sizeof(cModule),      /* tp_basicsize */
        0,                         /* tp_itemsize */
        call_destructor<cModule>,  /* tp_dealloc */
        0,                         /* tp_print */
        0,                         /* tp_getattr */
        0,                         /* tp_setattr */
        0,                         /* tp_reserved */
        0,                         /* tp_repr */
        0,                         /* tp_as_number */
        0,                         /* tp_as_sequence */
        0,                         /* tp_as_mapping */
        0,                         /* tp_hash  */
        0,                         /* tp_call */
        0,                         /* tp_str */
        0,                         /* tp_getattro */
        0,                         /* tp_setattro */
        0,                         /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,        /* tp_flags */
        0,                         /* tp_doc */
        0,                         /* tp_traverse */
        0,                         /* tp_clear */
        0,                         /* tp_richcompare */
        0,                         /* tp_weaklistoffset */
        0,                         /* tp_iter */
        0,                         /* tp_iternext */
        Methods,                   /* tp_methods */
        0,                         /* tp_members */
        0,                         /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        module_init,                /* tp_init */
        0,                         /* tp_alloc */
        PyType_GenericNew          /* tp_new */
};

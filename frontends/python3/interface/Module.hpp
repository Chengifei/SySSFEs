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

#ifndef ATOM_CODEGEN_SUPPORT_CVARIABLE_HPP
#define ATOM_CODEGEN_SUPPORT_CVARIABLE_HPP
#include <python_common.hpp>
#include <LLVM/driver.hpp>
#include <LLVM/diagnostics.hpp>
#include <sstream>

struct cModule : PyObject, Program {
    std::ostringstream str_out;
    DiagCtl diag{str_out};
    PyObject* output;
    cModule() {}
    ~cModule() {
        // NOBUG: FIXOTHERS: c++ destructors may be called even when class
        // invariants are not satisifed, i.e. due to failures in ctor.
        // Hence the XDECREF
        Py_XDECREF(output);
    }
};

extern PyTypeObject cModule_type;
#endif

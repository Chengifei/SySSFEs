/* Copyright 2017-2018 by Yifei Zheng
 *
 * Please avoid bare macro definition, and always use #define ABC (value).
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

#ifndef ATOM_CODEGEN_SUPPORT_CEXPR_HPP
#define ATOM_CODEGEN_SUPPORT_CEXPR_HPP
#include <python_common.hpp>
#include <support/Expr.hpp>
#include <memory>

enum OP {
    PLUS = 0,
    MINUS,
    MUL,
    DIV,
    POW
};

struct cExpr : PyObject {
    bool view;
    support::Expr* tree;
    ~cExpr() {
        if (!view)
            delete tree;
    }
};

PyObject* cExpr_getattr(PyObject* self, char* attr_name);
int cExpr_setattr(PyObject* self, char* attr_name, PyObject* val);
PyObject* cExpr_repr(PyObject* self);
int cExpr_init(PyObject* self, PyObject* args, PyObject*);

extern PyTypeObject cExprType;

#endif

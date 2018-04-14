/* Copyright 2018 by Yifei Zheng
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

#ifndef OPERATORS_HPP
#define OPERATORS_HPP
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <vector> 
#include <support/Expr.hpp>
#include "fcn_base.hpp"

struct op_info {
    llvm::Instruction* (*impl)(llvm::Function*, std::vector<llvm::Value*>&, const fcn_base&);
    void (*visitor)(fcn_base&, support::Expr::Op&) = nullptr;
    static void init(llvm::LLVMContext& c, llvm::Module& m);
    llvm::Instruction* call(llvm::Function* fcn, std::vector<llvm::Value*>& stack, const fcn_base& am) {
        return impl(fcn, stack, am);
    }
    void visit(fcn_base& a, support::Expr::Op& o) {
        if (visitor)
            visitor(a, o);
    }
};

extern op_info PLUS_OP;
extern op_info MINUS_OP;
extern op_info MUL_OP;
extern op_info DIV_OP;
extern op_info POW_OP;
extern op_info DIFF_OP;
#endif

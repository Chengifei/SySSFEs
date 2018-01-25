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
 *
 * This header creates a wrapper class for operators with underlying function,
 * which shall finally be optimized out. It shall be modified if doesn't.
 */

#ifndef OPERATORS_HPP
#define OPERATORS_HPP
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <vector>

struct op_info {
    llvm::Function* impl;
    llvm::CallInst* call(std::vector<llvm::Value*>& stack, unsigned argc) {
        // FIXME: not efficient
        std::vector<llvm::Value*> args(stack.end() - argc, stack.end());
        stack.erase(stack.end() - argc, stack.end());
        return llvm::CallInst::Create(impl, args);
    }
};

extern op_info PLUS_OP;
extern op_info MINUS_OP;
extern op_info MUL_OP;
extern op_info DIV_OP;
extern op_info POW_OP;

void init(llvm::LLVMContext& c, llvm::Module& m);
#endif

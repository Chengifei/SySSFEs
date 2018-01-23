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
 * This header defines codegen, which generates LLVM IR from math
 * exprsesions.
 */

#include <support/Expr.hpp>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Constants.h>
#include <llvm/ADT/StringMap.h>
#include <cstdlib>
#include "operators.hpp"

enum : long long {
    LLVM_CONST = 1,
    LLVM_ARG = 2,
    OP = -1
};

llvm::Function* codegen(llvm::LLVMContext& c, support::Expr& expr) {
    auto* dbl_tp = llvm::Type::getDoubleTy(c);
    llvm::StringMap<llvm::Argument*> args;

    char* ptr;
    for (support::Expr& it : support::Expr_preorder_iter(expr))
        if (it.type >= 0) { // ignore operators
            if (double d = std::strtod(it.data, &ptr); ptr != it.data) {
                it.data = reinterpret_cast<char*>(llvm::ConstantFP::get(dbl_tp, d));
                it.type = LLVM_CONST;
            }
            else {
                args.try_emplace(it.data, nullptr);
                it.type = LLVM_ARG;
            }
        }

    llvm::FunctionType* tp = llvm::FunctionType::get(dbl_tp, std::vector<llvm::Type*>(args.size(), dbl_tp), false);
    llvm::Function* func = llvm::Function::Create(tp, llvm::GlobalValue::ExternalLinkage);
    auto arg_it = func->arg_begin();

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(c, "", func);
    std::vector<llvm::Value*> stack;

    for (const support::Expr& it : support::Expr_const_postorder_iter(expr)) {
        if (it.type < 0) {
            op_info& info = *static_cast<op_info*>(it.op->op_data);
            llvm::Instruction* ret = info.call(stack);
            bb->getInstList().push_back(ret);
            stack.resize(stack.size() - it.op->argc);
            stack.push_back(ret); // Push the return value into stack
        }
        else if (it.type == LLVM_CONST)
            stack.push_back(reinterpret_cast<llvm::Value*>(const_cast<char*>(it.data)));
        else {
            llvm::Argument* arg;
            if (auto pair = args.find(it.data); pair != args.end())
                arg = pair->second;
            else {
                arg = arg_it++;
                args[it.data] = arg;
            }
            stack.push_back(arg);
        }
    }
    bb->getInstList().push_back(llvm::ReturnInst::Create(c, stack.back()));
    return func;
}

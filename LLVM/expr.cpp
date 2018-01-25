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
#include <operator_map.hpp>

enum : long long {
    LLVM_CONST = 1,
    LLVM_ARG = 2,
    OP = -1
};

op_info& get_op(const support::Expr& node) {
    if (static_cast<NODE_TYPE>(node.type) == NODE_TYPE::FUNC) {
        std::string name(std::move(*static_cast<std::string*>(node.op->op_data)));
        delete static_cast<std::string*>(node.op->op_data);
        // TODO
    }
    else {
        switch (static_cast<OPCODE>(
            reinterpret_cast<std::intptr_t>(node.op->op_data))) {
            case OPCODE::PLUS:
                return PLUS_OP;
            case OPCODE::MINUS:
                return MINUS_OP;
            case OPCODE::MUL:
                return MUL_OP;
            case OPCODE::DIV:
                return DIV_OP;
            case OPCODE::POW:
                return POW_OP;
        }
    }
}

// Make sure this function frees all memory allocated by parser.
llvm::Function* codegen(llvm::LLVMContext& c, support::Expr& expr) {
    auto* dbl_tp = llvm::Type::getDoubleTy(c);
    llvm::StringMap<llvm::Argument*> args;

    char* ptr;
    for (support::Expr& it : support::Expr_preorder_iter(expr))
        if (it.type >= 0) { // ignore operators
            const std::string* str = reinterpret_cast<const std::string*>(it.data);
            if (double d = std::strtod(str->c_str(), &ptr); ptr != str->c_str()) {
                it.data = reinterpret_cast<char*>(llvm::ConstantFP::get(dbl_tp, d));
                it.type = LLVM_CONST;
                delete str; // agree with what has been done in the parser
            }
            else {
                args.try_emplace(*str, nullptr); // this leave the actual arg uninitialized
                it.type = LLVM_ARG;
                // NOT DELETED, because we still need this to locate arg
            }
        }

    llvm::FunctionType* tp = llvm::FunctionType::get(dbl_tp, std::vector<llvm::Type*>(args.size(), dbl_tp), false);
    llvm::Function* func = llvm::Function::Create(tp, llvm::GlobalValue::ExternalLinkage);
    auto arg_it = func->arg_begin();

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(c, "", func);
    std::vector<llvm::Value*> stack;

    // we don't use a const iterator because we're deleting arg fields
    for (const support::Expr& it : support::Expr_postorder_iter(expr)) {
        if (it.type < 0) {
            op_info& info = get_op(it);
            llvm::Instruction* ret = info.call(stack, it.op->argc);
            bb->getInstList().push_back(ret);
            stack.push_back(ret); // Push the return value into stack
        }
        else if (it.type == LLVM_CONST)
            stack.push_back(reinterpret_cast<llvm::Value*>(const_cast<char*>(it.data)));
        else {
            llvm::Argument* arg;
            auto& name = *reinterpret_cast<const std::string*>(it.data);
            if (auto pair = args.find(name);
                pair != args.end() && pair->second) // make sure pair->second is initialized
                arg = pair->second;
            else {
                arg = arg_it++;
                pair->second = arg;
            }
            stack.push_back(arg);
            delete &name;
        }
    }
    bb->getInstList().push_back(llvm::ReturnInst::Create(c, stack.back()));
    return func;
}

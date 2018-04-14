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

#include "operators.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include "driver.hpp"
using namespace llvm;

Function* pow_func;

static Instruction* plus_handler(llvm::Function*, std::vector<Value*>& stack, const fcn_base&) {
    auto ret = BinaryOperator::CreateFAdd(*(stack.end() - 2), *(--stack.end()));
    stack.erase(stack.end() - 2, stack.end());
    return ret;
}

static Instruction* minus_handler(llvm::Function*, std::vector<Value*>& stack, const fcn_base&) {
    auto ret = BinaryOperator::CreateFSub(*(stack.end() - 2), *(--stack.end()));
    stack.erase(stack.end() - 2, stack.end());
    return ret;
}

static Instruction* mul_handler(llvm::Function*, std::vector<Value*>& stack, const fcn_base&) {
    auto ret = BinaryOperator::CreateFMul(*(stack.end() - 2), *(--stack.end()));
    stack.erase(stack.end() - 2, stack.end());
    return ret;
}

static Instruction* div_handler(llvm::Function*, std::vector<Value*>& stack, const fcn_base&) {
    auto ret = BinaryOperator::CreateFDiv(*(stack.end() - 2), *(--stack.end()));
    stack.erase(stack.end() - 2, stack.end());
    return ret;
}

static Instruction* pow_handler(llvm::Function*, std::vector<Value*>& stack, const fcn_base&) {
    std::vector<Value*> args(stack.end() - 2, stack.end());
    stack.erase(stack.end() - 2, stack.end());
    return CallInst::Create(pow_func, args);
}

static void diff_visitor(fcn_base& arg, ::support::Expr::Op&) {
    static_cast<function_builder&>(arg).request(ARG_REQ::HISTORY_ITERATOR_TO_SINGLE_FIELD);
}

static Instruction* diff_handler(llvm::Function* fcn, std::vector<Value*>&, const fcn_base& fb) {
    fcn_info arg(fb, fcn);
    auto iter_info = arg.get_iter();
    Value* iter = iter_info.val;
    Value* iprev = CallInst::Create(iter_info.info.dec, { iter }, "", &fcn->back());
    Value* inext = CallInst::Create(iter_info.info.inc, { iter }, "", &fcn->back());
    Value* vprev = CallInst::Create(iter_info.info.deref, { iprev }, "", &fcn->back());
    Value* vnext = CallInst::Create(iter_info.info.deref, { inext }, "", &fcn->back());
    Value* ydiff = BinaryOperator::CreateFSub(vnext, vprev, "", &fcn->back());
}

op_info PLUS_OP{plus_handler};
op_info MINUS_OP{minus_handler};
op_info MUL_OP{mul_handler};
op_info DIV_OP{div_handler};
op_info POW_OP{pow_handler};
op_info DIFF_OP{diff_handler, diff_visitor};

void op_info::init(LLVMContext& c, llvm::Module& m) {
    // Requires types to be initialized first
    // FIXME: enforce that
    Type* dbl_tp = Type::getDoubleTy(c);
    FunctionType* bin = FunctionType::get(dbl_tp, std::vector<Type*>(2, dbl_tp), false);
    pow_func = Function::Create(bin, GlobalVariable::ExternalLinkage, "pow", &m);
}


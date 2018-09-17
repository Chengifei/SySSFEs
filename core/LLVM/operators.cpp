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
#include "fcn_builder.hpp"
#include "operator_map.hpp"
using namespace llvm;

llvm::StringMap<op_info*> function_map{ {"diff", &DIFF_OP} };

static Function* pow_func;

static Value* plus_handler(std::vector<Value*>& stack, const fcn_builder&, BasicBlock* bb) {
    auto ret = BinaryOperator::CreateFAdd(*(stack.end() - 2), *(--stack.end()));
    stack.erase(stack.end() - 2, stack.end());
    bb->getInstList().push_back(ret);
    return ret;
}

static Value* minus_handler(std::vector<Value*>& stack, const fcn_builder&, BasicBlock* bb) {
    auto ret = BinaryOperator::CreateFSub(*(stack.end() - 2), *(--stack.end()));
    stack.erase(stack.end() - 2, stack.end());
    bb->getInstList().push_back(ret);
    return ret;
}

static Value* mul_handler(std::vector<Value*>& stack, const fcn_builder&, BasicBlock* bb) {
    auto ret = BinaryOperator::CreateFMul(*(stack.end() - 2), *(--stack.end()));
    stack.erase(stack.end() - 2, stack.end());
    bb->getInstList().push_back(ret);
    return ret;
}

static Value* div_handler(std::vector<Value*>& stack, const fcn_builder&, BasicBlock* bb) {
    auto ret = BinaryOperator::CreateFDiv(*(stack.end() - 2), *(--stack.end()));
    stack.erase(stack.end() - 2, stack.end());
    bb->getInstList().push_back(ret);
    return ret;
}

static Value* pow_handler(std::vector<Value*>& stack, const fcn_builder&, BasicBlock* bb) {
    std::vector<Value*> args(stack.end() - 2, stack.end());
    stack.erase(stack.end() - 2, stack.end());
    auto ret = CallInst::Create(pow_func, args);
    bb->getInstList().push_back(ret);
    return ret;
}

static Value* diff_handler(std::vector<Value*>& stack, const fcn_builder& fb, BasicBlock* bb) {
    std::vector<Value*> forward_args;
    forward_args.reserve(fb.size());
    forward_args.insert(forward_args.cend(), stack.end() - fb.size(), stack.end());
    Function* fcn = static_cast<Function*>(*(stack.end() - fb.size() - 1));
    stack.erase(stack.end() - fb.size() - 1, stack.end());
    auto y1 = CallInst::Create(fcn, forward_args, "", bb);
    for (Value* iter : fb.all_iters(forward_args.data()))
        CallInst::Create(fb.typeinfo.iter_dec[0], ArrayRef(&iter, 1), "", bb);
    auto y2 = CallInst::Create(fcn, forward_args, "", bb);
    auto ydiff = BinaryOperator::Create(Instruction::BinaryOps::FSub, y2, y1, "", bb);
    bb->getInstList().push_back(ydiff);
    return ydiff;
}

op_info PLUS_OP{plus_handler};
op_info MINUS_OP{minus_handler};
op_info MUL_OP{mul_handler};
op_info DIV_OP{div_handler};
op_info POW_OP{pow_handler};
op_info DIFF_OP{diff_handler, true, true};

void op_info::init(LLVMContext& c, llvm::Module& m) {
    Type* dbl_tp = Type::getDoubleTy(c);
    FunctionType* bin = FunctionType::get(dbl_tp, std::vector<Type*>(2, dbl_tp), false);
    pow_func = Function::Create(bin, GlobalVariable::ExternalLinkage, "pow", &m);
}


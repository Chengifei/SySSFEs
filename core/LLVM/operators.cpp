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
#include <vector>

using namespace llvm;

op_info PLUS_OP;
op_info MINUS_OP;
op_info MUL_OP;
op_info DIV_OP;
op_info POW_OP;
op_info DIFF_OP; // TODO

void init(LLVMContext& c, Module& m) {
    Type* dbl_tp = Type::getDoubleTy(c);
    FunctionType* bin = FunctionType::get(dbl_tp, std::vector<Type*>(2, dbl_tp), false);
{
    Function* plus = Function::Create(bin, GlobalVariable::PrivateLinkage, "__plus_dbl", &m);
    Argument* arg1 = plus->arg_begin();
    Argument* arg2 = arg1 + 1;
    BasicBlock* bb = BasicBlock::Create(c);
    plus->getBasicBlockList().push_back(bb);
    Instruction* plus_ret = BinaryOperator::CreateFAdd(arg1, arg2);
    bb->getInstList().push_back(plus_ret);
    bb->getInstList().push_back(ReturnInst::Create(c, plus_ret));
    PLUS_OP = op_info{plus};
}
{
    Function* minus = Function::Create(bin, GlobalVariable::PrivateLinkage, "__minus_dbl", &m);
    Argument* arg1 = minus->arg_begin();
    Argument* arg2 = arg1 + 1;
    BasicBlock* bb = BasicBlock::Create(c);
    minus->getBasicBlockList().push_back(bb);
    Instruction* minus_ret = BinaryOperator::CreateFSub(arg1, arg2);
    bb->getInstList().push_back(minus_ret);
    bb->getInstList().push_back(ReturnInst::Create(c, minus_ret));
    MINUS_OP = op_info{minus};
}
{
    Function* mul = Function::Create(bin, GlobalVariable::PrivateLinkage, "__mul_dbl", &m);
    Argument* arg1 = mul->arg_begin();
    Argument* arg2 = arg1 + 1;
    BasicBlock* bb = BasicBlock::Create(c);
    mul->getBasicBlockList().push_back(bb);
    Instruction* mul_ret = BinaryOperator::CreateFMul(arg1, arg2);
    bb->getInstList().push_back(mul_ret);
    bb->getInstList().push_back(ReturnInst::Create(c, mul_ret));
    MUL_OP = op_info{mul};
}
{
    Function* div = Function::Create(bin, GlobalVariable::PrivateLinkage, "__div_dbl", &m);
    Argument* arg1 = div->arg_begin();
    Argument* arg2 = arg1 + 1;
    BasicBlock* bb = BasicBlock::Create(c);
    div->getBasicBlockList().push_back(bb);
    Instruction* div_ret = BinaryOperator::CreateFDiv(arg1, arg2);
    bb->getInstList().push_back(div_ret);
    bb->getInstList().push_back(ReturnInst::Create(c, div_ret));
    DIV_OP = op_info{div};
}
{
    Function* pow = Function::Create(bin, GlobalVariable::ExternalLinkage, "pow", &m);
    POW_OP = op_info{pow};
}
}


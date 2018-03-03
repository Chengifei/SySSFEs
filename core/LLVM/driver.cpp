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

#include "driver.hpp"
#include <llvm/IR/Value.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include "optimize.hpp"
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
    LLVM_MEM = 3,
    OP = -1
};

llvm::StringMap<op_info*> function_map{ {"diff", &DIFF_OP} };

op_info& get_op(const support::Expr& node) {
    if (static_cast<NODE_TYPE>(node.type) == NODE_TYPE::FUNC) {
        std::string name(std::move(*static_cast<std::string*>(node.op->op_data)));
        delete static_cast<std::string*>(node.op->op_data);
        if (auto it = function_map.find(name); it != function_map.end())
            return *it->second;
        throw; // FIXME
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

struct member_accessor {
    std::string instn, fn; // instance name, field name
};

// return StringMap to unique the result;
llvm::StringMap<llvm::Type*> Module::convert_tree(support::Expr& expr) {
    // convert tree to adopt LLVM specific concepts, with the format specified in the enum
    auto* dbl_tp = llvm::Type::getDoubleTy(TheContext);
    char* ptr; // for the use of strtod
    llvm::StringMap<llvm::Type*> args;
    for (support::Expr& it : support::Expr_preorder_iter(expr))
        if (it.type >= 0) { // ignore operators
            std::string* str = reinterpret_cast<std::string*>(it.data);
            if (double d = std::strtod(str->c_str(), &ptr); ptr != str->c_str()) {
                it.data = reinterpret_cast<char*>(llvm::ConstantFP::get(dbl_tp, d));
                it.type = LLVM_CONST;
                delete str; // agree with what has been done in the parser
            }
            else if (str->front() == '$') {
                std::size_t pos = str->find('.', 2);
                std::string fn(str->substr(pos + 1));
                str->resize(pos);
                str->erase(0, 1);
                args.try_emplace(*str, object_type.get_ptr_type());
                it.data = reinterpret_cast<char*>(new member_accessor{std::move(*str), std::move(fn)});
                delete str;
                it.type = LLVM_MEM;
            }
            else {
                // FIXME: Use actual type
                args.try_emplace(*str, dbl_tp); // this leave the actual arg uninitialized
                it.type = LLVM_ARG;
                // NOT DELETED, because we still need this to locate arg
            }
        }
    return args;
}

// Make sure this function frees all memory allocated by parser.
llvm::Function* Module::codegen(support::Expr& expr) {
    
    auto* dbl_tp = llvm::Type::getDoubleTy(TheContext);
    llvm::StringMap<llvm::Type*> type_map(convert_tree(expr));

    std::vector<llvm::Type*> arg_tp;
    std::vector<std::string> arg_names;
    arg_tp.reserve(type_map.size());
    arg_names.reserve(arg_tp.size());
    for (auto& arg : type_map) {
        arg_tp.push_back(arg.second);
        arg_names.push_back(arg.first()); // should move this, but no way
    }
    
    llvm::FunctionType* tp = llvm::FunctionType::get(dbl_tp, arg_tp, false);
    llvm::Function* func = llvm::Function::Create(tp, llvm::GlobalValue::ExternalLinkage);

    llvm::StringMap<llvm::Argument*> args;
    {
    auto name_it = arg_names.begin();
    for (auto arg_it = func->arg_begin(); arg_it != func->arg_end(); ++arg_it, ++name_it) {
        args[std::move(*name_it)] = arg_it;
        // for "object"s, set attribute dereferenceable.
    }
    }

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(TheContext, "", func);
    std::vector<llvm::Value*> stack;

    llvm::IntegerType* idx_type = llvm::IntegerType::get(TheContext, 32);

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
        else if (it.type == LLVM_MEM) {
            auto& ma = *reinterpret_cast<member_accessor*>(it.data);
            llvm::Argument* arg = args[ma.instn];
            std::vector<llvm::Value*> indices{llvm::ConstantInt::get(idx_type, 0),
                                              llvm::ConstantInt::get(idx_type, object_type.get_mem_idx(ma.fn))};
            auto* gep = llvm::GetElementPtrInst::CreateInBounds(arg, indices);
            bb->getInstList().push_back(gep);
            bb->getInstList().push_back(new llvm::LoadInst(gep));
            stack.push_back(&bb->getInstList().back());
            delete &ma;
        }
        else {
            llvm::Argument* arg;
            auto name = reinterpret_cast<std::string*>(it.data);
            arg = args[*name];
            stack.push_back(arg);
            delete name;
        }
    }
    bb->getInstList().push_back(llvm::ReturnInst::Create(TheContext, stack.back()));
    return func;
}

void Module::reg_mem(const support::type& tp, const char* name) {
    object_type.add(name, tp);
}

std::size_t Module::reg_rule(support::Expr&& expr) {
    std::size_t ret = rules.size();
    rules.push_back(std::move(expr));
    return ret;
}

int Module::write(const char* fn) {
    initialize();
    for (support::Expr& expr : rules) {
        llvm::Function* f = codegen(expr);
        f->setName(mangler::rule_name{}("rule", f->getFunctionType()));
        TheModule.getFunctionList().push_back(f);
    }

    if (llvm::verifyModule(TheModule, &llvm::errs()))
        return 1;

    TheModule.setTargetTriple(llvm::sys::getDefaultTargetTriple());
    std::string err;

    optimizer opt(2, 2);
    opt(TheModule);

    std::error_code EC;
    llvm::raw_fd_ostream os(fn, EC, llvm::sys::fs::F_Excl);
    machine_code_generator::initialize();
    machine_code_generator mcg(llvm::sys::getDefaultTargetTriple());
    mcg(TheModule, os);
    return 0;
}

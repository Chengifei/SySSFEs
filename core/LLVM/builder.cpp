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
#include "operators.hpp"
#include "operator_map.hpp"
#include "mangling.hpp"
#include <algorithm>
#include <numeric>
#include <cstdlib>
#include <memory>

enum : long long {
    LLVM_CONST = 1,
    LLVM_ARG = 2,
    LLVM_MEM = 3,
    OP = -1
};

// Currently I don't have a good solution to enforce function calling
// convetnions. If process_args is to be updated, fcn_info in driver.hpp
// shall also be updated.
llvm::FunctionType* Builder::process_args(const fcn_base& fb) {
    auto* dbl = llvm::Type::getDoubleTy(mod.Context);
    std::vector<llvm::Type*> arg_tps;
    arg_tps.reserve(fb.tps.size() + fb.combs + std::accumulate(fb.reqs, fb.reqs + fcn_base::req_size::value, 0));
    // argument are in the sequence of named arguments, objects, and requests
    arg_tps.insert(arg_tps.end(), fb.tps.cbegin(), fb.tps.cend());
    arg_tps.insert(arg_tps.end(), fb.combs, types.obj);
    if (fb.reqs[static_cast<std::size_t>(ARG_REQ::HISTORY_ITERATOR_TO_SINGLE_FIELD)]) {
        arg_tps.push_back(make_iter_type().tp);
    }
    if (fb.reqs[static_cast<std::size_t>(ARG_REQ::HISTORY_BEGIN_END_W_FMT)])
        ;
    if (fb.reqs[static_cast<std::size_t>(ARG_REQ::RAW_OFFSET_STRAND)])
        ; // arg_tps.push_back(off);
    if (fb.reqs[static_cast<std::size_t>(ARG_REQ::COMBINATION)])
        ;
    return llvm::FunctionType::get(dbl, arg_tps, false);
}

llvm::Function* Builder::make_func(const fcn_base& fb) {
    return llvm::Function::Create(process_args(fb), llvm::GlobalValue::ExternalLinkage);
}

fcn_base Builder::pre_pass(support::Expr& expr) {
    function_builder args(types);
    // convert tree to adopt LLVM specific concepts, with the format specified in the enum
    auto* dbl_tp = llvm::Type::getDoubleTy(mod.Context);
    char* ptr; // for the use of strtod
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
                args.request_obj(types.obj, *str);
                it.data = reinterpret_cast<char*>(new member_accessor{std::move(*str),  prog.object_type.get_mem_idx(fn)});
                delete str;
                it.type = LLVM_MEM;
            }
            else {
                // FIXME: Use actual type
                args.request(dbl_tp, *str);
                it.type = LLVM_ARG;
                // NOT DELETED, because we still need this to locate arg
            }
        }
        else // if (it.type == static_cast<long long>(NODE_TYPE::OP))
            static_cast<op_info*>(it.op->op_data)->visit(args, *it.op, ctx);
    return std::move(args);
}

const Builder::chunk_type& Builder::make_chunk_type() {
    // as this is called in ctor, use assert instead
    // guaranteed to be initialized
    if (!types.chunk) {
        llvm::Type* iter = llvm::StructType::create(mod.Context);
        llvm::Type* head = mod.get_ptr_type(iter);
        llvm::Type* array = llvm::ArrayType::get(types.obj, chunk_len());
        types.chunk = llvm::StructType::create(mod.Context, { head, array, head });
    }
    return types.chunk;
}

const Builder::iterator_type& Builder::make_iter_type() {
    using namespace llvm;
    types.iter.tp = StructType::get(mod.get_ptr_type(types.chunk), mod.get_ptr_type(types.chunk));
    auto inc_dec_tp = llvm::FunctionType::get(llvm::Type::getVoidTy(mod.Context), {types.iter.tp}, false);
    IntegerType* idx_type = IntegerType::get(mod.Context, 32);
    Constant* one = ConstantInt::get(idx_type, 1);
    types.iter.inc = llvm::Function::Create(inc_dec_tp, GlobalValue::ExternalLinkage, "iter_inc", &mod.llvm_module);
    {
        Argument* arg = types.iter.inc->arg_begin();
        BasicBlock* bb = &types.iter.inc->getEntryBlock();
        auto cur = ExtractValueInst::Create(arg, {0}, "", bb);
        auto new_cur = GetElementPtrInst::CreateInBounds(cur, std::vector<Value*>{one}, "", bb);
        auto cur_chunk = ExtractValueInst::Create(arg, {1}, "", bb);
        auto chunk_data_end = GetElementPtrInst::CreateInBounds(cur_chunk,
            std::vector<Value*>{ConstantInt::get(idx_type, chunk_len())}, "", bb);
        auto cmp = CmpInst::Create(Instruction::ICmp, CmpInst::Predicate::ICMP_EQ, new_cur, chunk_data_end);
        BasicBlock::Create(mod.Context, "", types.iter.inc);
    }
    types.iter.dec = llvm::Function::Create(inc_dec_tp, GlobalValue::ExternalLinkage, "iter_dec", &mod.llvm_module);
    auto deref_tp = llvm::FunctionType::get(types.obj, {types.iter.tp}, false);
    types.iter.deref = llvm::Function::Create(deref_tp, GlobalValue::ExternalLinkage, "", &mod.llvm_module);
    return types.iter;
}

// Make sure this function frees all memory allocated by parser.
llvm::Function* Builder::codegen(const fcn_base& fb, support::Expr& expr) {
    llvm::Function* fcn = make_func(fb);
    fcn_info args(fb, fcn);

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(mod.Context, "", fcn);
    std::vector<llvm::Value*> stack;

    llvm::IntegerType* idx_type = llvm::IntegerType::get(mod.Context, 32);

    // we don't use a const iterator because we're deleting arg fields
    for (const support::Expr& it : support::Expr_postorder_iter(expr)) {
        if (it.type < 0) {
            llvm::Instruction* ret = static_cast<op_info*>(it.op->op_data)->call(fcn, stack, fb);
            bb->getInstList().push_back(ret);
            stack.push_back(ret); // Push the return value into stack
        }
        else if (it.type == LLVM_CONST)
            stack.push_back(reinterpret_cast<llvm::Value*>(const_cast<char*>(it.data)));
        else if (it.type == LLVM_MEM) {
            std::unique_ptr<member_accessor> ma(reinterpret_cast<member_accessor*>(it.data));
            // LLVM_MEM have essentially two cases, an named instance_name or unnamed instance_name
            llvm::Argument* arg;
            try {
                int instn = std::stoi(ma->instn);
                arg = args[instn];
            }
            catch (std::invalid_argument&) {
                arg = args[ma->instn];
            }
            unsigned indices[] = {ma->fn};
            auto val = llvm::ExtractValueInst::Create(arg, indices, "", bb);
            stack.push_back(val);
        }
        else {
            std::unique_ptr<std::string> name(reinterpret_cast<std::string*>(it.data));
            llvm::Argument* arg = args[*name];
            stack.push_back(arg);
        }
    }
    bb->getInstList().push_back(llvm::ReturnInst::Create(mod.Context, stack.back()));
    return fcn;
}

llvm::StructType* Module::reg_type(const composite_decl& decl) {
    std::vector<llvm::Type*> llvm_tps;
    llvm_tps.reserve(decl.get().mem_types.size());
    for (const auto& i : decl.get().mem_types)
        llvm_tps.push_back(get_llvm_type(i));
    return llvm::StructType::get(Context, llvm_tps);
}

llvm::PointerType* Module::get_ptr_type(llvm::Type* ptr) {
    return llvm::PointerType::getUnqual(ptr);
}

bool Module::verify() {
    return llvm::verifyModule(llvm_module, &llvm::errs());
}

int Builder::compile() {
    for (support::Expr& expr : prog.rules) {
        llvm::Function* f = codegen(pre_pass(expr), expr);
        f->setName(mangler::rule_name{}("rule", f->getFunctionType()));
        mod.llvm_module.getFunctionList().push_back(f);
    }
    return mod.verify();
}

int Builder::write(const char* fn) {
    optimizer opt(2, 2);
    opt(mod.llvm_module);

    std::error_code EC;
    llvm::raw_fd_ostream os(fn, EC, llvm::sys::fs::F_Excl);
    machine_code_generator mcg(mod.llvm_module.getTargetTriple());
    mcg(mod.llvm_module, os);
    return 0;
}

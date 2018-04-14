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

#ifndef LLVM_DRIVER_HPP
#define LLVM_DRIVER_HPP
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include "Program.hpp"
#include <support/type.hpp>
#include "operators.hpp"
#include "machine_gen.hpp"
#include "initialization.hpp"
#include "type_cache.hpp"
#include <type_traits>
#include <vector>
#include <numeric>

struct Module {
    llvm::LLVMContext Context;
    llvm::Module llvm_module{"ATOM", Context};
    llvm::DataLayout dl{&llvm_module};

    llvm::StructType* reg_type(const composite_decl&);
    llvm::Type* get_llvm_type(const support::type& tp) {
        llvm::Type* base_tp = nullptr;
        switch (tp.base) {
            case support::type::REAL:
                base_tp = llvm::Type::getDoubleTy(Context);
                break;
            case support::type::INT:
                base_tp = llvm::Type::getInt64Ty(Context);
                break;
            case support::type::BUFFER:
                return nullptr;
        }
        if (tp.agg > 1)
            return llvm::ArrayType::get(base_tp, tp.agg);
        return base_tp;
    }
    llvm::PointerType* get_ptr_type(llvm::Type*);
    bool verify();
};

struct fcn_base;

class Builder {
    Program& prog;
    Module& mod;
    type_cache types {};
    typedef type_cache::iterator_type iterator_type;
    typedef type_cache::combination_type combination_type;
    typedef type_cache::chunk_type chunk_type;
public:
#pragma GCC visibility push(default)
    Builder(Program& p, Module& m) : prog(p), mod(m) {
        // FIXME: Memory leak if called multiple times
        op_info::init(mod.Context, mod.llvm_module);
        machine_code_generator::initialize();
        mod.llvm_module.setTargetTriple(llvm::sys::getDefaultTargetTriple());
        types.obj = mod.reg_type(prog.object_type);
        make_chunk_type();
    }
    // FIXME: This probably shouldn't be compile-time constant
    typedef std::integral_constant<std::size_t, 1 << 20> chunk_size;
    int compile();
    int write(const char* fn); // 0 on success
#pragma GCC visibility pop
public:
    std::size_t chunk_len() const {
        return (chunk_size::value - mod.dl.getPointerSize() * 2) / mod.dl.getTypeAllocSize(types.obj);
    }
private:
    const chunk_type& make_chunk_type();
    const iterator_type& make_iter_type();
    llvm::PointerType* hist_type();
    const combination_type& make_comb_type();
    fcn_base pre_pass(support::Expr&);
    llvm::FunctionType* process_args(const fcn_base&);
    llvm::Function* make_func(const fcn_base&);
    llvm::Function* codegen(const fcn_base&, support::Expr&);
};

class fcn_info {
    const fcn_base& from;
    llvm::Function* fcn;
    template <typename T>
    struct ret_t {
        const T& info;
        llvm::Value* val;
    };
    typedef type_cache::iterator_type iterator_type;
    typedef type_cache::combination_type combination_type;
    typedef type_cache::chunk_type chunk_type;
public:
    fcn_info(const fcn_base& b, llvm::Function* fcn) : from(b), fcn(fcn) {}
    llvm::Argument* operator[](const std::string& str) {
        std::size_t index = std::find(from.names.cbegin(), from.names.cend(), str) - from.names.cbegin();
        return fcn->arg_begin() + index;
    }
    ret_t<iterator_type> get_iter() {
        std::size_t acc = std::accumulate(from.reqs, from.reqs + 
            static_cast<std::size_t>(ARG_REQ::HISTORY_ITERATOR_TO_SINGLE_FIELD) + 1, 0) - 1;
        return { from.typeinfo.iter, fcn->arg_begin() + (from.tps.size() + from.combs + acc) };
    }
    llvm::Argument* operator[](std::size_t comb_idx) {
        return fcn->arg_begin() + (from.tps.size() + comb_idx);
    }
};
#endif

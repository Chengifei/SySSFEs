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

#ifndef LLVM_TYPE_CACHE_HPP
#define LLVM_TYPE_CACHE_HPP
#include "composite_decl.hpp"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <vector>

struct env_t {
    llvm::StructType* obj;
    struct combination_type {
        llvm::Type* tp;
        llvm::Function* inc;
    } comb;
    typedef llvm::StructType* chunk_type;
    chunk_type chunk;
    llvm::User* first_chunk;
    llvm::GlobalVariable* controls[8]{};
    llvm::Function* iter_inc[8]{};
    llvm::Function* iter_dec[8]{};
    llvm::Function* od_updater;
};

struct fcn_base;

struct Module {
    llvm::LLVMContext Context;
    llvm::Module llvm_module{"ATOM", Context};
    std::vector<fcn_base> func_infos;
    llvm::DataLayout dl{&llvm_module};

    llvm::StructType* reg_type(const composite_decl&);
    llvm::Type* get_llvm_type(const support::type& tp);
    llvm::PointerType* get_ptr_type(llvm::Type*);
    bool verify();
    int write(const char* fn);
    void add_function(fcn_base&&, llvm::Function*);
};
#endif

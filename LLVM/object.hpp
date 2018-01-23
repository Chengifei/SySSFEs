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
 * This header defines corresponding structure for object in ATOM.
 */

#ifndef OBJECT_HPP
#define OBJECT_HPP
#include <vector>
#include <string>
#include "types.hpp"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>

class object_node {
    llvm::LLVMContext& ctx;
    std::vector<std::string> mem_names;
    std::vector<llvm::Type*> mem_types;
public:
    object_node(llvm::LLVMContext& ctx) : ctx(ctx) {}
    void add(std::string&& name, const support::type& tp) {
        mem_names.push_back(std::move(name));
        mem_types.push_back(get_llvm_type(ctx, tp));
    }
    llvm::StructType* get_type() const {
        return llvm::StructType::get(ctx, llvm::ArrayRef<llvm::Type*>(mem_types));
    }
};
#endif

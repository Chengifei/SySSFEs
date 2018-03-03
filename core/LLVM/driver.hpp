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
#include "diagnostics.hpp"
#include "object.hpp"
#include <support/Expr.hpp>
#include "operators.hpp"
#include "mangling.hpp"
#include "machine_gen.hpp"
#include <iostream>

// FIXME: Module is de facto singleton, currently we call initialize
// inside write. If the user calls write multiple times, or the user
// creates multiple Modules, init function would overwrite op,
// resulting in memory leak.
class Module {
    DiagCtl diagnostic{std::cerr};
    llvm::LLVMContext TheContext;
    llvm::Module TheModule{"ATOM", TheContext};
    object_node object_type{TheContext};
    std::vector<support::Expr> rules;
public:
    void initialize() {
        init(TheContext, TheModule);
    }
    void reg_mem(const support::type&, const char*);
    std::size_t reg_rule(support::Expr&&);
    int write(const char* fn); // 0 on success
    support::Expr* get_rule(std::size_t i);
    void issue_diag(DiagCtl::LEVEL, const char*);
private:
    // do not call convert tree directly, it is designed to be called in codegen
    llvm::StringMap<llvm::Type*> convert_tree(support::Expr& expr);
    llvm::Function* codegen(support::Expr& expr);
};
#endif

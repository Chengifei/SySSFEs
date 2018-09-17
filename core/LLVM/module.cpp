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
#include "module.hpp"
#include "machine_gen.hpp"
#include "optimize.hpp"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>

llvm::Type* Module::get_llvm_type(const support::type& tp) {
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

int Module::write(const char* fn) {
    //llvm::errs() << mod.llvm_module;
    optimizer opt(2, 2);
    opt(llvm_module);

    std::error_code EC;
    llvm::raw_fd_ostream os(fn, EC, llvm::sys::fs::F_Excl);
    machine_code_generator mcg(llvm_module.getTargetTriple());
    mcg(llvm_module, os);
    return 0;
}

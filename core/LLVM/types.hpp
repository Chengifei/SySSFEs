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

#ifndef TYPES_HPP
#define TYPES_HPP
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <support/type.hpp>

// FIXME: Move this to correct place
static llvm::Type* get_llvm_type(llvm::LLVMContext& c, const support::type& tp) {
    llvm::Type* base_tp = nullptr;
    switch (tp.base) {
        case support::type::REAL:
            base_tp = llvm::Type::getDoubleTy(c);
            break;
        case support::type::INT:
            base_tp = llvm::Type::getInt64Ty(c);
            break;
        case support::type::BUFFER:
            return nullptr;
    }
    if (tp.agg > 1)
        return llvm::ArrayType::get(base_tp, tp.agg);
    return base_tp;
}
#endif

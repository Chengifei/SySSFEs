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
#include "fcn_base.hpp"
#include "module.hpp"
#include "composite_decl.hpp"
#include <llvm/IR/LLVMContext.h>

std::size_t fcn_base::size() const {
    return tmp_reqs.size() + cmb_reqs.size() + obj_reqs.size() + std::count(reqs, reqs + fcn_base::req_size::value, true);
}

// Currently I don't have a good solution to enforce function calling
// conventions. If make_functype is updated, fcn_builder shall also be updated.
llvm::FunctionType* fcn_base::make_functype(llvm::LLVMContext& ctx) const {
    auto* dbl = llvm::Type::getDoubleTy(ctx);
    std::vector<llvm::Type*> arg_tps;
    arg_tps.reserve(size());
    // argument are in the sequence of named arguments, objects, and requests
    arg_tps.insert(arg_tps.end(), tmp_reqs.size(), dbl);
    arg_tps.insert(arg_tps.end(), cmb_reqs.size(), typeinfo.obj);
    arg_tps.insert(arg_tps.end(), obj_reqs.size(), typeinfo.obj);
    if (reqs[static_cast<std::size_t>(fcn_base::HISTORY_BEGIN_END_W_FMT)])
        ;
    if (reqs[static_cast<std::size_t>(fcn_base::RAW_OFFSET_STRAND)])
        ; // arg_tps.push_back(off);
    return llvm::FunctionType::get(dbl, arg_tps, false);
}

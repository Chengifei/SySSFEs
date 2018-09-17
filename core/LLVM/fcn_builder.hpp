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

#ifndef LLVM_FCN_BUILDER_HPP
#define LLVM_FCN_BUILDER_HPP
#include <llvm/IR/Function.h>
#include "module.hpp"
#include "fcn_base.hpp"
#include <iter_utils.hpp>

struct fcn_builder : fcn_base {
    typedef env_t::combination_type combination_type;
    typedef env_t::chunk_type chunk_type;
    std::size_t get_iter(support::id_type ma) const {
        return tmp_reqs.size() + cmb_reqs.size() + (obj_reqs.find(ma) - obj_reqs.begin());
    }
    std::size_t get_cmb_iter(support::id_type ca) const {
        return tmp_reqs.size() + (cmb_reqs.find(ca) - cmb_reqs.begin());
    }
    iter_utils::array_view<llvm::Value*> all_iters(llvm::Value** arg_begin) const {
        return iter_utils::array_view<llvm::Value*>{arg_begin + tmp_reqs.size(), arg_begin + size()};
    }
    std::size_t lookup_tmp(support::id_type id) const {
        return tmp_reqs.find(id) - tmp_reqs.begin();
    }
};
#endif

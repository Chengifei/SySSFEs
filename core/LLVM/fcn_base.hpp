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

#ifndef LLVM_FCN_BASE_HPP
#define LLVM_FCN_BASE_HPP
#include <type_traits>
#include <boost/container/flat_set.hpp>
#include <support/variable.hpp>
#include "accessors.hpp"

class composite_decl;
struct Program;
struct env_t;

namespace llvm {
    class LLVMContext;
    class FunctionType;
}

struct fcn_base {
    enum ARG_REQ {
        HISTORY_BEGIN_END_W_FMT = 0,
        RAW_OFFSET_STRAND,
        SIZE_INVALID // guard and a size calculator
    };
    const env_t& typeinfo;
    //const Program& prog;
    typedef std::integral_constant<std::size_t, 
        static_cast<std::size_t>(SIZE_INVALID)> req_size;
    // This is a potential optimization site but since it is still small,
    // it is ignored. If it grows to over 8, change to bit field.
    bool reqs[req_size::value] = {};
    // These three containers hold exact requests
    boost::container::flat_set<support::id_type> obj_reqs;
    boost::container::flat_set<support::id_type> cmb_reqs;
    boost::container::flat_set<support::id_type> tmp_reqs;
    // These two containers hold iterator requests
    boost::container::flat_set<support::id_type> obj_iters;
    boost::container::flat_set<support::id_type> cmb_iters;
    fcn_base(const env_t& tc) : typeinfo(tc) {}
    llvm::FunctionType* make_functype(llvm::LLVMContext&) const;
    std::size_t size() const;
};
#endif

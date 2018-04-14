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
#include <vector>
#include <type_traits>
#include "type_cache.hpp"

enum class ARG_REQ : char {
    HISTORY_ITERATOR_TO_SINGLE_FIELD = 0,
    HISTORY_BEGIN_END_W_FMT,
    RAW_OFFSET_STRAND,
    COMBINATION,
    SIZE_INVALID // guard and a size calculator
};

struct fcn_base {
    typedef std::integral_constant<std::size_t, 
        static_cast<std::size_t>(ARG_REQ::SIZE_INVALID)> req_size;
    // This is a potential optimization site but since it is still small,
    // it is ignored. If it grows to over 8, change to bit field.
    const type_cache& typeinfo;
    bool reqs[req_size::value] {};
    std::vector<llvm::Type*> tps;
    std::vector<std::string> names;
    std::size_t combs = 0; // maximum requests of combination
    explicit fcn_base(const type_cache& tc) : typeinfo(tc) {}
};

struct function_builder : fcn_base {
    using fcn_base::fcn_base;
    void request(ARG_REQ t) {
        reqs[static_cast<std::size_t>(t)] = true;
    }
    void request(llvm::Type* tp, std::string str) { // FIXME: copy str only when necessary
        if (auto it = std::find(names.cbegin(), names.cend(), str); it != names.cend()) {
            names.push_back(std::move(str));
            tps.push_back(tp);
        }
    }
   void request_obj(llvm::Type* obj_tp, std::string str) {
        try {
            int req = std::stoi(str);
            if (++req > combs)
                combs = req;
        }
        catch (std::invalid_argument&) {
            request(obj_tp, std::move(str));
        }
   }
};
#endif

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

#ifndef LLVM_COMPOSITE_DECL_HPP
#define LLVM_COMPOSITE_DECL_HPP
#include <vector>
#include <string>
#include <algorithm>
#include <support/type.hpp>

struct composite_decl_base {
    std::vector<std::string> mem_names;
    std::vector<support::type> mem_types;
protected:
    composite_decl_base() {}
};

class composite_decl : private composite_decl_base {
public:
    typedef std::size_t field_t;
    void add(std::string&& name, support::type tp) {
        mem_names.push_back(std::move(name));
        mem_types.push_back(tp);
    }
    field_t get_mem_idx(const std::string& str) const {
        // FIXME: Handle not found cases, this is unsafe.
        return std::find(mem_names.cbegin(), mem_names.cend(), str) - mem_names.cbegin();
    }
    const composite_decl_base& get() const {
        return *this;
    }
    std::size_t size() const {
        return mem_names.size();
    }
};
#endif

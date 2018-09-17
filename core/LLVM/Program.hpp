/* Copyright 2017-2018 by Yifei Zheng
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

#ifndef PROGRAM_HPP
#define PROGRAM_HPP
#include <vector>
#include <unordered_map>
#include <support/Expr.hpp>
#include "initialization.hpp"
#include "composite_decl.hpp"

struct Program {
    composite_decl object_type;
    VariablePool varpool;
    std::vector<support::Expr> rules;
private:
    std::unordered_map<std::string, support::id_type> sym_table;
    std::vector<support::id_type> obj_list;
public:
    template <typename... T>
    std::size_t add_var(std::string&& name, T&&... args) {
        // FIXME: Might have duplicates
        auto idx = varpool.vars.size() + atom_ctrl_size;
        sym_table[std::move(name)] = idx;
        return idx;
    }
    // This method has to be called after object_type has been fully defined
    template <typename... T>
    std::size_t add_obj(std::string&& name, T*... args) {
        auto idx = varpool.vars.size() + atom_ctrl_size;
        sym_table[name] = idx;
        obj_list.push_back(idx);
        for (const auto& i : object_type.get().mem_types)
            varpool.vars.emplace_back();
        return idx;
    }
    template <typename... T>
    std::size_t add_ctrl(std::string&& name, T&&... args) {
        auto idx = varpool.controls.size();
        sym_table[name] = idx;
        varpool.add_ctrl(std::move(name), std::forward<T&&>(args)...);
        return idx;
    }
    support::id_type get_base(support::id_type fn) const {
        return *std::lower_bound(obj_list.cbegin(), obj_list.cend(), fn);
    }
    support::id_type operator[](const std::string& n) const {
        return sym_table.at(n);
    }
    int canonicalize();
};
#endif

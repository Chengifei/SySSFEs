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

#ifndef INITIALIZATION_HPP
#define INITIALIZATION_HPP
#include <vector>
#include <unordered_map>
#include <support/variable.hpp>
#include <array_builder.hpp>

static constexpr std::size_t atom_ctrl_size = 8;

class VariablePool {
    support::ctrl_variable ctrls[atom_ctrl_size];
public:
    array_builder<support::ctrl_variable> controls{ &ctrls };
    std::vector<support::var_w_init> vars;
    void add_ctrl(std::string&&, double start, double step, double end);
    void add_ctrl(std::string&&, int start, int step, int end);
    void add_init(support::id_type id, const support::order_t& order, void* val) {
        if (!vars[id - atom_ctrl_size].inits.emplace(order, val).second)
            throw; // Overwriting an existing initializer
    }
    bool is_initialized(const support::var_designation& vd) const {
        if (is_control(vd.id))
            return true;
        else
            return vars[vd.id - atom_ctrl_size].inits.count(vd.order);
    }
    void* val_of(const support::var_designation& vd) const {
        return vars[vd.id - atom_ctrl_size].inits.at(vd.order);
    }
    bool is_control(support::id_type var_id) const {
        return var_id < atom_ctrl_size;
    }
};
#endif

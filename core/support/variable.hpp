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

#ifndef VARIABLE_HPP
#define VARIABLE_HPP
#include "type.hpp"
#include <string>
#include <array>
#include <unordered_map>

namespace support {

struct VariableBase {
    std::string name;
    type tp;
};

struct ctrl_variable : VariableBase {
    void* start;
    void* step;
    void* end;
};

typedef std::array<std::uint8_t, 8> order_t;
// FIXME: refactor 8 has to equal to VariablePool::ctrl_size

struct var_w_init : VariableBase {
private:
    struct hash_arr {
        std::size_t operator()(const order_t& arr) const {
            return *reinterpret_cast<const std::size_t*>(arr.data());
        }
    };
public:
    typedef std::unordered_map<order_t, void*, hash_arr> init_collection;
    init_collection inits;
};

typedef std::size_t id_type;

struct var_designation {
    id_type id;
    order_t order;
};

}
#endif

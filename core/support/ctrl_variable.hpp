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

#ifndef CTRL_VARIABLE_HPP
#define CTRL_VARIABLE_HPP
#include "type.hpp"
#include <type_traits>

namespace support {

typedef std::integral_constant<std::size_t, 8> ctrl_size;

struct CtrlVariable {
    support::type tp;
    void* start;
    void* step;
    void* end;
};

}
#endif

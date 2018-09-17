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

#ifndef ACCESSORS_HPP
#define ACCESSORS_HPP
#include "composite_decl.hpp"

struct cmb_accessor {
    std::size_t idx;
    composite_decl::field_t fn;
};

struct cmb_less {
    bool operator()(const cmb_accessor& l, const cmb_accessor& r) const {
        return l.idx < r.idx || (l.idx == r.idx && l.fn < r.fn);
    }
};
#endif

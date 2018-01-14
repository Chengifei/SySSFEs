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
 *
 * Unordered set support as well as set algorithms.
 */

#ifndef SET_SUPPORT_HPP
#define SET_SUPPORT_HPP
#include <unordered_set>
#include <boost/functional/hash.hpp>

template <typename T, class hash = boost::hash<T>>
using CSF_set = std::unordered_set<T, hash>;

template <class C1, class C2>
bool is_subset(const C1& S1, const C2& S2) {
    for (const auto& i : S1) {
        if (!S2.count(i))
            return false;
    }
    return true;
}

template <class C1, class C2>
C1 union_sets(const C1& S1, const C2& S2) {
    C1 ret = S1;
    for (auto& i: S2)
        ret.insert(i);
    return ret;
}

template <class C1, class C2>
C1 intersect_sets(const C1& S1, const C2& S2) {
    C1 ret;
    for (const auto& i: S2)
        if (S1.count(i))
            ret.insert(i);
    return ret;
}

template <class C1, class C2>
void exclude(C1& S1, const C2& S2) noexcept {
    for (const auto& i: S2)
        S1.erase(i);
}

template <class C, class T>
bool verify_then_remove(C& set, const T& key) {
    auto it = set.find(key);
    if (it != set.end()) {
        set.erase(it);
        return true;
    }
    return false;
}

template <class C, class T>
bool verify_then_insert(C& set, T key) {
    return set.insert(std::move(key)).second;
}
#endif

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
 * This file defines a few data types to represent rules.
 */

#ifndef RULE_TYPES_HPP
#define RULE_TYPES_HPP
#include <cstddef>
#include <vector>
#include <array>
#include <memory>
#include <algorithm>
#include <boost/container/flat_map.hpp>
#include <iter_utils.hpp>
#include <support/variable.hpp>

namespace codegen {

struct var_status {
    bool need_update;
public:
    const bool initialized;
    constexpr var_status(bool, bool initialized) // FIXME
        : need_update(true), initialized(initialized) {}
    bool update() {
        if (!need_update)
            return false;
        need_update = false;
        return true;
    }
    bool needs_update() const {
        return need_update;
    }
};

// ### CAUTION ### CAUTION ### CAUTION ### CAUTION ### CAUTION
// Although resolver module uses the same var_designation struct
// as with LLVM, its id field has absolutely different meanings.
// For LLVM, id is assigned to every 'programmatical' variable,
// while in codegen, it is assigned to each basic field of object
// as well as independent controls and global variables which have
// to have types of int or float.
using support::var_designation, support::order_t, support::id_type;

struct Rule : std::vector<var_designation> {
    bool enabled = true;
    using std::vector<var_designation>::vector;
};

struct step {
    std::unique_ptr<std::size_t[]> rule;
    std::size_t* rule_end;
    std::unique_ptr<var_designation[]> var;
    std::size_t size() const {
        return rule_end - rule.get();
    }
};

typedef std::vector<step> resolved_sequence;

struct variable_pool {
    typedef var_designation key_type;
    typedef var_status mapped_type;
private:
    struct packed_vars {
        std::vector<order_t> orders;
        std::vector<var_status> states;
        var_status& operator[](const order_t& o) {
            return *(states.begin() +
                (std::find(orders.cbegin(), orders.cend(), o) - orders.cbegin()));
        }
        template <typename... T>
        bool add(order_t o, T&&... args) {
            auto it = std::find(orders.cbegin(), orders.cend(), o);
            if (it != orders.cend())
                return false; // for debugging, assert the two values are the same
            orders.push_back(o);
            states.emplace_back(std::forward<T&&>(args)...);
            return true;
        }
    };
    typedef boost::container::flat_map<id_type, packed_vars> pool_t;
    pool_t pool;
public:
    var_status& operator[](const var_designation& v) {
        return pool[v.id][v.order];
    }
    std::pair<std::vector<var_status>::iterator, std::vector<var_status>::iterator>
    at(id_type base) {
        std::vector<var_status>& all = pool.at(base).states;
        return std::make_pair(all.begin(), all.end());
    }
    std::size_t base_size() const {
        return pool.size();
    }
    template <typename... T>
    bool add(const var_designation& v, T&&... args) {
        auto it = pool.find(v.id);
        if (it != pool.cend())
            return it->second.add(v.order, std::forward<T&&>(args)...);
        else
            return pool.emplace_hint(it, v.id, packed_vars{})->second.add(v.order, std::forward<T&&>(args)...);
    }
    struct iterator {
        typedef var_status value_type;
        typedef value_type& reference;
        typedef value_type* pointer;
        typedef std::ptrdiff_t difference_type;
        typedef std::bidirectional_iterator_tag iterator_category;
    public: // FIXME
        pool_t::iterator major;
        const pool_t::iterator last_major;
        std::vector<var_status>::iterator minor;
    public:
        reference operator*() noexcept {
            return *minor;
        }
        pointer operator->() noexcept {
            return &operator*();
        }
        iterator& operator++() noexcept {
            ++minor;
            if (minor == major->second.states.end()) {
                ++major;
                minor = major != last_major ? major->second.states.begin()
                                            : std::vector<var_status>::iterator{};
            }
            return *this;
        }
        iterator operator++(int) noexcept {
            iterator it = *this;
            operator++();
            return it;
        }
        bool operator==(const iterator& rhs) noexcept {
            return major == rhs.major && minor == rhs.minor;
        }
        bool operator!=(const iterator& rhs) noexcept {
            return !operator==(rhs);
        }
    };
public:
    iterator begin() {
        return iterator{ pool.begin(), pool.end(), pool.begin()->second.states.begin() };
    }
    iterator end() {
        return iterator{ pool.end(), pool.end(), {} };
    }
};

class RuleResolver {
private:
    variable_pool& pool;
    iter_utils::array_view<Rule> pack;
    resolved_sequence sln;
public:
    RuleResolver(variable_pool& p, Rule* rbegin, Rule* rend)
        : pool(p), pack{rbegin, rend} {}
    resolved_sequence get();
    bool solve();
private:
    enum : int { // return value enum
        SUCCEEDED_AND_UPDATED,
        SUCCEEDED_NONE_UPDATED,
        FAILED
    };
    /// The function may fail, and return false on fail.
    int alg_consistent(bool);
    int broadcast(const id_type&) noexcept;
};

}
#endif

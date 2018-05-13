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
#include <support/variable.hpp>
#include <cstddef>
#include <vector>
#include <memory>
#include <algorithm>
#include <boost/container/flat_map.hpp>
#include <iter_utils.hpp>

namespace codegen {

class variable {
    bool need_update = true;
public:
    const bool initialized;
    constexpr variable(bool need_update, bool initialized)
        : need_update(need_update), initialized(initialized) {}
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

struct Rule : std::vector<support::variable_designation> {
    bool enabled = true;
    using std::vector<support::variable_designation>::vector;
};

struct step {
    std::unique_ptr<std::size_t[]> rule;
    std::size_t* rule_end;
    std::unique_ptr<support::variable_designation[]> var;
};

typedef std::vector<step> resolved_sequence;

struct variable_pool {
    typedef support::variable_designation key_type;
    typedef variable mapped_type;
private:
    struct packed_vars {
        std::vector<support::order_t> orders;
        std::vector<variable> states;
        variable& operator[](const support::order_t& o) {
            return *(states.begin() +
                (std::find(orders.cbegin(), orders.cend(), o) - orders.cbegin()));
        }
        template <typename... T>
        bool add(support::order_t o, T&&... args) {
            auto it = std::find(orders.cbegin(), orders.cend(), o);
            if (it != orders.cend())
                return false; // for debugging, assert the two values are the same
            orders.push_back(o);
            states.emplace_back(std::forward<T&&>(args)...);
            return true;
        }
    };
    typedef boost::container::flat_map<support::base_t, packed_vars> pool_t;
    pool_t pool;
public:
    variable& operator[](const support::variable_designation& v) {
        return pool[v.id][v.order];
    }
    iter_utils::array_view<variable> at(support::base_t base) {
        std::vector<variable>& all = pool.at(base).states;
        return {&*all.begin(), &*all.end()};
    }
    template <typename... T>
    bool add(const support::variable_designation& v, T&&... args) {
        auto it = pool.find(v.id);
        if (it != pool.cend())
            return it->second.add(v.order, std::forward<T&&>(args)...);
        else
            return pool.emplace_hint(it, v.id, packed_vars{})->second.add(v.order, std::forward<T&&>(args)...);
    }
    struct iterator {
        typedef variable value_type;
        typedef value_type& reference;
        typedef value_type* pointer;
        typedef std::ptrdiff_t difference_type;
        typedef std::bidirectional_iterator_tag iterator_category;
    public: // FIXME
        pool_t::iterator major;
        const pool_t::iterator last_major;
        std::vector<variable>::iterator minor;
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
                                            : std::vector<variable>::iterator{};
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
    int broadcast(const support::base_t&) noexcept;
};

}
#endif

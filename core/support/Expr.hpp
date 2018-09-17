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
 * This header defines the class Expr.
 */

#ifndef SUPPORT_EXPR_HPP
#define SUPPORT_EXPR_HPP
#include <iter_utils.hpp>
#include <vector>
#include <stack>
#include <cassert>

namespace support {

/// Expr does not assume the ownership of data
struct Expr {
    enum : long long {
        OP = -1
    };
    struct Op {
        void* op_data;
        std::vector<Expr> args;
        Op(std::size_t argc, void* op_data)
            : op_data(op_data) {
            args.resize(argc);
        }
    };
    long long type; /// negative type means has children
    union {
        char* data; /// raw data at users discretion
        Op* op;
    };
    Expr() : type(0) {}
    explicit Expr(Expr::Op* o) : type(OP), op(o) {}
    Expr(const Expr&) = delete;
    Expr& operator=(const Expr&) = delete;
    Expr(Expr&& rhs) : type(rhs.type), data(rhs.data) {
        rhs.type = 0;
        rhs.data = 0;
    }
    Expr& operator=(Expr&& rhs) {
        type = rhs.type;
        data = rhs.data;
        rhs.type = 0;
        rhs.data = 0;
        return *this;
    }
    ~Expr() {
        if (type < 0)
            delete op;
    }
};

template <typename T>
struct Expr_postorder_iter_impl :
    iter_utils::non_trivial_end_iter<Expr_postorder_iter_impl<T>> {
    std::stack<T*, std::vector<T*>> stack;
    T* current;
    Expr_postorder_iter_impl(T& expr)
        : current(&expr) {
        while (current->type < 0) {
            stack.push(current);
            current = &current->op->args[0];
        }
    }
    void operator++() {
        if (stack.empty()) {
            current = nullptr;
            return;
        }
        ++current;
        if (current - stack.top()->op->args.data() < stack.top()->op->args.size()) {
            while (current->type < 0) {
                stack.push(current);
                current = &current->op->args[0];
            }
        }
        else {
            current = stack.top();
            stack.pop();
        }
    }
    T& operator*() const {
        return *current;
    }
    bool exhausted() const {
        return !current;
    }
};

typedef Expr_postorder_iter_impl<Expr> Expr_postorder_iter;
typedef Expr_postorder_iter_impl<const Expr> Expr_const_postorder_iter;

template <typename T>
struct Expr_preorder_iter_impl :
    iter_utils::non_trivial_end_iter<Expr_preorder_iter_impl<T>> {
    T* const base;
    T* current;
    std::stack<T*, std::vector<T*>> stack;
    Expr_preorder_iter_impl(T& expr) : base(&expr), current(&expr) {}
    void operator++() {
        if (current->type < 0) {
            stack.push(current);
            current = current->op->args.data();
        }
        else {
            ++current;
            while (!stack.empty() && current - stack.top()->op->args.data() == stack.top()->op->args.size()) {
                current = stack.top() + 1;
                stack.pop();
            }
        }
    }
    T& operator*() const {
        return *current;
    }
    bool exhausted() const {
        return current != base && stack.empty();
    }
};

typedef Expr_preorder_iter_impl<Expr> Expr_preorder_iter;
typedef Expr_preorder_iter_impl<const Expr> Expr_const_preorder_iter;

/// Allows RAII functors to be applied in a recursion-like fashion
template <typename T, class F>
struct Expr_custom_iter_impl :
    iter_utils::non_trivial_end_iter<Expr_preorder_iter_impl<T>> {
    T* const base;
    T* current;
    std::stack<T*, std::vector<T*>> stack;
    std::stack<T*, std::vector<T*>> estack;
    std::stack<F, std::vector<F>> fstack;
    Expr_custom_iter_impl(T& expr) : base(&expr), current(&expr) {}
    void operator++() {
        if (current->type < 0) {
            stack.push(current);
            current = current->op->args.data();
        }
        else {
            ++current;
            while (!stack.empty() && current - stack.top()->op->args.data() == stack.top()->op->args.size()) {
                if (!estack.empty() && stack.top() == estack.top()) {
                    fstack.pop();
                    estack.pop();
                }
                current = stack.top() + 1;
                stack.pop();
            }
        }
    }
    T& operator*() const {
        return *current;
    }
    bool exhausted() const {
        return current != base && stack.empty();
    }
    template <typename... A>
    void emplace(A&&... args) {
        assert(current->type < 0);
        estack.push(current);
        fstack.emplace(std::forward<A&&>(args)...);
    }
};

template <class T>
using Expr_custom_iter = Expr_custom_iter_impl<Expr, T>;
template <class T>
using Expr_custom_const_iter = Expr_custom_iter_impl<const Expr, T>;

typedef Expr_preorder_iter Expr_fast_iter; // FIXME: Placeholder typedef
typedef Expr_const_preorder_iter Expr_fast_const_iter;
}
#endif

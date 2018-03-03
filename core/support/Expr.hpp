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
#include <memory>
#include <iter_utils.hpp>
#include <stack>
namespace support {

/// Expr does not assume the ownership of data
struct Expr {
    enum : long long {
        OP = -1
    };
    struct Op {
        const std::size_t argc;
        void* op_data;
        std::unique_ptr<Expr[]> args;
        Op(std::size_t argc, void* op_data)
            : argc(argc), op_data(op_data) {
            args = std::make_unique<Expr[]>(argc);
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

struct Expr_const_postorder_iter :
    iter_utils::non_trivial_end_iter<Expr_const_postorder_iter> {
    std::stack<std::pair<const Expr*, std::size_t>> stack;
    const Expr* current;
    Expr_const_postorder_iter(const Expr& expr)
        : current(&expr) {
        while (current->type < 0) {
            stack.emplace(current, 0);
            current = &current->op->args[0];
        }
    }
    void operator++() {
        if (stack.empty()) {
            current = nullptr;
            return;
        }
        if (stack.top().second + 1< stack.top().first->op->argc) {
            ++current;
            ++stack.top().second;
            while (current->type < 0) {
                stack.emplace(current, 0);
                current = &current->op->args[0];
            }
        }
        else {
            current = stack.top().first;
            stack.pop();
        }
    }
    const Expr& operator*() const {
        return *current;
    }
    bool exhausted() {
        return !current;
    }
};

struct Expr_postorder_iter :
    iter_utils::non_trivial_end_iter<Expr_const_postorder_iter> {
    std::stack<std::pair<Expr*, std::size_t>> stack;
    Expr* current;
    Expr_postorder_iter(Expr& expr)
        : current(&expr) {
        while (current->type < 0) {
            stack.emplace(current, 0);
            current = &current->op->args[0];
        }
    }
    void operator++() {
        if (stack.empty()) {
            current = nullptr;
            return;
        }
        if (stack.top().second + 1< stack.top().first->op->argc) {
            ++current;
            ++stack.top().second;
            while (current->type < 0) {
                stack.emplace(current, 0);
                current = &current->op->args[0];
            }
        }
        else {
            current = stack.top().first;
            stack.pop();
        }
    }
    Expr& operator*() const {
        return *current;
    }
    bool exhausted() {
        return !current;
    }
};

struct Expr_preorder_iter :
    iter_utils::non_trivial_end_iter<Expr_preorder_iter> {
    std::stack<std::pair<Expr::Op*, std::size_t>> stack;
    Expr* current;
    Expr_preorder_iter(Expr& expr) : current(&expr) {
        if (current->type < 0)
            stack.emplace(current->op, 0);
    }
    void operator++() {
        if (stack.empty()) {
            current = nullptr;
            return;
        }
        current = stack.top().first->args.get() + stack.top().second++;
        if (current->type < 0)
            stack.emplace(current->op, 0);
        while (!stack.empty() && stack.top().second == stack.top().first->argc) {
            stack.pop();
        }
    }
    Expr& operator*() const {
        return *current;
    }
    bool exhausted() {
        return !current;
    }
};

}
#endif

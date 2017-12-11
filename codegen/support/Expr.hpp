/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This header defines the class Expr.
 */
#include <memory>
#include "../iter_utils.hpp"
#include <stack>

/// Expr does not assume the ownership of data
struct Expr {
    enum : long long {
        OP = -1,
        DATA = 0
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
        const char* data; /// raw data at users discretion
        Op* op;
    };
    Expr() {}
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

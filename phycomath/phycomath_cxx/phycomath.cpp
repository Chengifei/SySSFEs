/* Copyright (c) 2016-2017 Yifei Zheng
 * All rights reserved.
 * Unauthorized copying of this file, in any means is strictly prohibited.
 */
#include "phycomath.hpp"
#include "math_utils.hpp"
#include <stack>
#include <experimental/string_view>

inline bool is_alpha(const std::string& str) noexcept {
    for (char i : str) {
        if (!(('a' <= i  && i <= 'z') || ('A' <= i && i <= 'Z')))
            return false;
    }
    return true;
}

// There are strong evidence that shows manually written function are
// dramatically faster than it by regex.
//!
//! Check if str can be converted to double or int
//!
inline int is_double(const std::string& str) noexcept {
    unsigned dotcount = 0;
    for (char i : str) {
        if ('0' <= i && i <= '9') {
        }
        else if (i == '.')
            ++dotcount;
        else //failure not dot or digit
            return 0;
    }
    switch (dotcount) {
        case 1:
            return 1;
        case 0:
            return 2;
        default:
            return 0;
    }
}

inline int STOI(string s) noexcept {
    int x = 0;
    for (auto i : s)
        x = (x * 10) + (i - '0');
    return x;
}


namespace math {

const Op* get_op(const string& str) {
    switch (str2int(str.c_str())) {
        case str2int("+"):
            return &operators::PLU_OP;
        case str2int("-"):
            return &operators::MIN_OP;
        case str2int("*"):
            return &operators::MUL_OP;
        case str2int("/"):
            return &operators::DIV_OP;
        case str2int("^"):
            return &operators::POW_OP;
    }
    return nullptr;
}

Op::Op(string name, unsigned priority, func_t func, bool infix) :
    name(name), priority(priority), func(func), minfix(infix) {}

#ifdef PY_EXT
string Op::repr() const noexcept {
    return "Operator '" + name + "' with priority "
        + std::to_string(priority);
}
#endif

namespace operators {

const map<string, const Op*> built_in_dict
{ { "sin", &SIN_OP }, {"cos", &COS_OP}, {"tan", &TAN_OP}, {"ln", &LN_OP} , {"exp", &EXP_OP}, {"sqrt", &SQRT_OP} };

std::set<string> infixlist{ "(", ")", "+", "-", "*", "/", "^" };
std::set<string> bilist{};

void init() noexcept {
    for (auto i : built_in_dict) {
        bilist.insert(i.first);
    }
}

} // namespace operators


struct token_iterator {
    std::experimental::string_view str;
    string token;
    string delim;
    token_iterator(const string& str) : str(str) { operator++(); }
    token_iterator& operator++() noexcept {
        token.clear();
        if (!delim.empty())
            std::swap(token, delim);
        else {
            while (!str.empty()) {
                delim = _inc_impl(str, "+", "-", "*", "/", "^", "(", ")");
                if (!delim.empty()) {
                    str.remove_prefix(delim.size());
                    if (token.empty()) {
                        std::swap(token, delim);
                    }
                    break;
                }
                else {
                    token += str[0];
                    str.remove_prefix(1);
                }
            }
        }
        return *this;
    }
    string& operator*() noexcept {
        return token;
    }
    string& operator->() noexcept {
        return token;
    }
private:
    string _inc_impl(decltype(str)) noexcept {
        return "";
    }
    template <class C, class... R>
    string _inc_impl(std::experimental::string_view& str,C c, R... _R) noexcept {
        if (match(c))
            return c;
        else
            return _inc_impl(str, _R...);
    }
    bool match(const char* cstr) noexcept {
      	auto begin = cstr;
      	auto it = str.begin();
        while (*begin != 0 && it != str.end()) {
          	if (*begin != *it) {
              	return false;
            }
          	++it;
          	++begin;
        }
        return true;
    }
};

class parse_manager {
    expression_tree tree;
    expression_tree::post_order_iterator cursor = tree._post_order_root();
    std::stack<decltype(cursor)> stack;
    void append(int arg) noexcept {
        tree.insert_as_child(cursor, arg_t(arg));
    }
    void append(double arg) noexcept {
        tree.insert_as_child(cursor, arg_t(arg));
    }
    void append(string arg) noexcept {
        tree.insert_as_child(cursor, arg_t(std::move(arg)));
        cursor->args.back()->var = true;
    }
    void append(const Op* op) noexcept {
        if (tree.as_op(cursor->content)->priority > op->priority) {
            if (!cursor->parent)
                cursor = tree.insert_as_parent(cursor, arg_t(op));
            else {
                cursor = expression_tree::post_order_iterator(cursor->parent);
                append(op);
            }
        }
        else if (tree.as_op(cursor->content)->priority == op->priority) {
            if (tree.as_op(cursor->content) != op)
                cursor = tree.insert_as_parent(cursor, arg_t(op));
        }
        else {
            auto last_child = cursor.get_at(cursor->args.size() - 1);
            cursor = tree.insert_as_child(cursor, arg_t(op));
            tree.move_child(last_child, cursor);
        }
    }
    void appendL() noexcept {
        stack.push(cursor);
        cursor = tree.insert_as_child(cursor, arg_t(&operators::PLU_OP));
        tree.insert_as_child(cursor, arg_t(0));
    }
    void appendR() noexcept {
        cursor = stack.top();
        stack.pop();
        if (operators::bilist.count(tree.as_op(cursor->content)->name))
            cursor = expression_tree::post_order_iterator(cursor->parent);
    }
    void append_built_in(const Op* op) noexcept {
        cursor = tree.insert_as_child(cursor, arg_t(op));
    }
public:
    parse_manager() {
        cursor = tree.insert_as_parent(tree._post_order_root(),
                                       arg_t(&operators::PLU_OP));
    }
    void append_branch(string str) {
        if (str[0] == '(') { // str[0] is the whole string, for performance
            appendL();
        }
        else if (str[0] == ')') {
            appendR();
        }
        else {
            int i = is_double(str);
            if (i == 1)
                append(std::stod(str));
            else if (i == 2)
                append(STOI(str));
            else if (operators::infixlist.count(str)) {
                append(get_op(str));
            }
            else if (operators::bilist.count(str)) {
                append_built_in(operators::built_in_dict.at(str));
            }
            else if (is_alpha(str))
                append(str);
            else
                throw std::runtime_error(
                    str + ": Not a valid variable name");
        }
    }
    auto get_tree() noexcept {
        for (const auto& i : tree.citer()) {
            if (i.var && i.parent) {
                auto temp = i.parent;
                do {
                    temp->var = true;
                } while ((temp = temp->parent) && !temp->var);
            }
        }
        return std::move(tree);
    }
};

expression_tree parse(string input) {
    parse_manager a;
    for (token_iterator it(input); *it != ""; ++it) {
        a.append_branch(std::move(*it));
    }
    return a.get_tree();
}

eqn_t::eqn_t(expression_tree&& lhs, expression_tree&& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

void eqn_t::group_to_left() {
    auto temp = lhs.mroot.get();
    lhs.insert_as_parent(lhs._post_order_root(), arg_t(&operators::MIN_OP));
    lhs.move_tree(std::move(rhs), lhs._post_order_root());
    if (temp->var || rhs.mroot->var)
        lhs.mroot->var = true;
    lhs.slot_available = false;
}
#ifdef PY_EXT
double eqn_t::fNsolve(string varname, PyObject* dict) {
    if (rhs.mroot)
        group_to_left();
    return solver::fNsolve(lhs, varname, _PyDict_to_UMap(dict));
}
#endif

} // namespace math

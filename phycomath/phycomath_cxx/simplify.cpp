#include "expression_tree.hpp"
#include <boost/functional/hash.hpp>
#include "math_utils.hpp"

namespace math::simplify {

inline arg_t maybe_ftoi(double d) {
    if (double_equal_abs_only(d, (int) d))
        return arg_t((int) d);
    else
        return arg_t(d);
}

struct rule_base {
    // CAUTION: DO NOT EVER INSTANTIATE FROM BASE CLASS
    using node_t = expression_tree::node;
    using post_order_iterator = expression_tree::post_order_iterator;
    expression_tree& tree;
    rule_base(expression_tree& tree) noexcept : tree(tree) {}
protected:
    bool is_add(post_order_iterator it) {
        auto op = static_cast<const op_wrapper* const>(it->content.get_ptr())->get_ptr();
        return (!it->args.empty()) && (op == &operators::PLU_OP);
    }
    bool is_minus(post_order_iterator ptr) {
        auto op = static_cast<const op_wrapper* const>(ptr->content.get_ptr())->get_ptr();
        return (!ptr->args.empty()) && (op == &operators::MIN_OP);
    }
    bool is_mul(post_order_iterator it) {
        auto op = static_cast<const op_wrapper* const>(it->content.get_ptr())->get_ptr();
        return (!it->args.empty()) && (op == &operators::MUL_OP);
    }
    bool is_div(post_order_iterator ptr) {
        auto op = static_cast<const op_wrapper* const>(ptr->content.get_ptr())->get_ptr();
        return (!ptr->args.empty()) && (op == &operators::DIV_OP);
    }
    bool is_pow(post_order_iterator it) {
        auto op = static_cast<const op_wrapper* const>(it->content.get_ptr())->get_ptr();
        return (!it->args.empty()) && (op == &operators::POW_OP);
    }
    bool is_plain(post_order_iterator it) {
        return dynamic_cast<const int_wrapper*>(it->content.get_ptr()) ||
            dynamic_cast<const double_wrapper*>(it->content.get_ptr());
    }
    bool is_var(post_order_iterator it) {
        return dynamic_cast<const string_wrapper*>(it->content.get_ptr());
    }
};

struct conv_base : rule_base {
    using rule_base::rule_base;
    virtual post_order_iterator apply_on(post_order_iterator) = 0;
};

struct simp_base : rule_base {
    using rule_base::rule_base;
};

struct combine_like_terms : simp_base {
    using simp_base::simp_base;
    using ident = std::pair<string, double>;
    struct info {
        double coeff;
        post_order_iterator pos;
    };
    std::unordered_map<ident, std::vector<info>, boost::hash<ident>> data;
    void apply_on(std::vector<post_order_iterator>& args) {
        for (auto i : args) {
            if (is_var(i)) {
                auto varname = i->content.to_string();
                data[{ varname, 1 }].push_back({ 1,i }); //test
            }
            else
                search_var_and_get_coeff2(post_order_iterator(i));
        }
        for (auto pair : data) {
            double sum = 0;
            for (auto i : pair.second)
                sum += i.coeff;
            if (double_equal_abs_only(sum, 0)) {
                for (auto i : pair.second)
                    tree.erase(i.pos);
            }
            else if (double_equal(sum, 1))
                ;
            else if (double_equal(sum, -1))
                ;
            else {
                auto parent = args[0]->parent;
                for (auto i : pair.second)
                    tree.erase(i.pos);
                auto cursor = tree.insert_as_child(post_order_iterator(parent), arg_t(&operators::MUL_OP));
                tree.insert_as_child(cursor, maybe_ftoi(sum));
                tree.insert_as_child(cursor, arg_t(pair.first.first));
            }
        }
    }
private:
    void search_var_and_get_coeff2(post_order_iterator it) {
        if (is_minus(it) && it->args.size() == 2) {
            if (is_var(it.get_at(1))) {
                auto varname = it.get_at(1)->content.to_string();
                insert<ident, info>({ { varname,1 },{ -1 ,it } });
            }
        }
        else if (is_mul(it) && it->args.size() == 2) {
            if (is_var(it.get_at(1)) && is_plain(it.get_at(0))) {
                auto varname = it.get_at(1)->content.to_string();
                insert<ident, info>({ { varname,1 },
                { it.get_at(0)->content.to_double(map<string, double>{}) ,it } });
            }
        }
        else if (is_div(it) && it->args.size() == 2) {
            if (is_var(it.get_at(0)), is_plain(it.get_at(1))) {
                auto varname = it.get_at(0)->content.to_string();
                insert<ident, info>({ { varname,1 },
                {1 / it.get_at(0)->content.to_double(map<string, double>{}) ,it } });
            }
        }
    }
    template <typename A, typename B>
    void insert(std::pair<A, B> val) {
        if (data.count(val.first))
            data[val.first].push_back(val.second);
        else
            data.insert({ val.first,{val.second} });
    }
};

struct combine_like_terms_minus_first : simp_base {
    using simp_base::simp_base;
    using ident = std::pair<string, double>;
    struct info {
        double coeff;
        post_order_iterator pos;
    };
    std::unordered_map<ident, std::vector<info>, boost::hash<ident>> data;
    void apply_on(post_order_iterator it) {
        auto iter = it->args.cbegin();
        while (!is_var(post_order_iterator(iter->get())))
            ++iter;
        // speicialize for first arg
        ident minuend = { "", 0 };
        post_order_iterator first(iter->get());
        ++iter;
        if (is_var(first)) {
            minuend = { first->content.to_string(), 1 };
            data[minuend].push_back({ 1, first });
        }
        else if (is_minus(first) && first->args.size() == 2) {
            if (is_var(first.get_at(1))) {
                minuend = { first.get_at(1)->content.to_string(), 1 };
                data[minuend].push_back({ -1 , first });
            }
        }
        else if (is_mul(first) && first->args.size() == 2) {
            if (is_var(first.get_at(1)) && is_plain(first.get_at(0))) {
                minuend = { first.get_at(1)->content.to_string(), 1 };
                data[minuend].push_back({ first.get_at(0)->content.to_double(map<string, double>{}), first });
            }
        }
        else if (is_div(first) && first->args.size() == 2) {
            if (is_var(first.get_at(0)), is_plain(first.get_at(1))) {
                minuend = { first.get_at(0)->content.to_string(), 1 };
                data[minuend].push_back({ 1 / first.get_at(0)->content.to_double(map<string, double>{}), first });
            }
        }
        for (; iter != it->args.cend() ; ++iter) {
            if (is_var(post_order_iterator(iter->get()))) {
                auto varname = (iter->get())->content.to_string();
                data[{ varname, 1 }].push_back({ 1, post_order_iterator(iter->get()) }); //test
            }
            else
                search_var_and_get_coeff2(post_order_iterator(iter->get()));
        }
        if (minuend != ident({ "", 0 })) {
            auto working = data[minuend];
            double sum = working[0].coeff;
            for (auto it = ++working.begin(); it != working.end(); ++it)
                sum -= it->coeff;
            if (double_equal_abs_only(sum, 0)) {
                for (auto i : working)
                    tree.erase(i.pos);
            }
            else if (double_equal(sum, 1)) {
                for (auto it = ++working.begin(); it != working.end(); ++it)
                    tree.erase(it->pos);
                tree.replace_existing_node(working[0].pos, working[0].pos.get_at(1));
            }
            else if (double_equal(sum, -1)) {
                for (auto it = ++working.begin(); it != working.end(); ++it)
                    tree.erase(it->pos);
                tree.insert_child_at(
                    tree.insert_as_parent(working[0].pos, arg_t(&operators::MIN_OP)), 0,
                    arg_t(0));
            }
            else {
                for (auto i : working)
                    tree.erase(i.pos);
                auto cursor = tree.insert_as_child(first, arg_t(&operators::MUL_OP));
                tree.insert_as_child(cursor, maybe_ftoi(sum));
                tree.insert_as_child(cursor, arg_t(minuend.first));
            }
        }
    }
private:
    void search_var_and_get_coeff2(post_order_iterator it) {
        if (is_minus(it) && it->args.size() == 2) {
            if (is_var(it.get_at(1))) {
                auto varname = it.get_at(1)->content.to_string();
                insert<ident, info>({ { varname,1 },{ -1 ,it } });
            }
        }
        else if (is_mul(it) && it->args.size() == 2) {
            if (is_var(it.get_at(1)) && is_plain(it.get_at(0))) {
                auto varname = it.get_at(1)->content.to_string();
                insert<ident, info>({ { varname,1 },
                { it.get_at(0)->content.to_double(map<string, double>{}) ,it } });
            }
        }
        else if (is_div(it) && it->args.size() == 2) {
            if (is_var(it.get_at(0)), is_plain(it.get_at(1))) {
                auto varname = it.get_at(0)->content.to_string();
                insert<ident, info>({ { varname,1 },
                { 1 / it.get_at(0)->content.to_double(map<string, double>{}) ,it } });
            }
        }
    }
    template <typename A, typename B>
    void insert(std::pair<A, B> val) {
        if (data.count(val.first))
            data[val.first].push_back(val.second);
        else
            data.insert({ val.first,{ val.second } });
    }
};

struct add_reduce : conv_base {
    using conv_base::conv_base;
    post_order_iterator apply_on(post_order_iterator it) override {
        double arg = 0;
        {
            std::vector<post_order_iterator> give_up;
            for (unsigned i = 0; i != it->args.size();) {
                auto t = it.get_at(i);
                if (is_plain(t)) {
                    arg += tree.cast_to_double(it.get_at(i), {});
                    tree.erase(it.get_at(i));
                }
                else if (is_add(t)) {
                    transfer_children(it.get_at(i), it);
                    tree.erase(it.get_at(i));
                }
                else if (is_minus(t)) {
                    if (transfer_from_minus(it.get_at(i), it))
                        tree.erase(it.get_at(i));
                    else {
                        give_up.push_back(it.get_at(i));
                        ++i;
                    }
                }
                else {
                    give_up.push_back(it.get_at(i));
                    ++i;
                }
            }
            combine_like_terms(tree).apply_on(give_up);
        }
        if (it->args.empty())
            it->content = arg_t(arg);
        else if (it->args.size() == 1 && double_equal_abs_only(arg, 0))
            it = tree.replace_existing_node(it, it.get_at(0));
        else if (!double_equal_abs_only(arg, 0) || it->args.size() == 1)
            tree.insert_as_child(it, arg_t(arg));
        // RESET FLAGS AND RELEASE MEMORY
        return ++it;
    }
private:
    void transfer_children(post_order_iterator from, post_order_iterator to) {
        to->args.reserve(from->args.size() + to->args.size());
        tree.move_children(to, from->args.begin(), from->args.end());
    }
    bool transfer_from_minus(post_order_iterator from, post_order_iterator to) {
        to->args.reserve(from->args.size() + to->args.size() - 1);
        bool all_transferred = true;
        tree.move_child(from.get_at(0), to);
        for (unsigned i = 0; i != from->args.size();) {
            if (negate(*from.get_at(i)))
                tree.move_child(from.get_at(i), to);
            else {
                all_transferred = false;
                ++i;
            }
        }
        if (!all_transferred || from->args.size() == 1)
            tree.insert_child_at(from, 0, arg_t(0));
        return all_transferred;
    }
    static bool negate(node_t& n) {
        if (auto ptr = dynamic_cast<int_wrapper* const>(n.content.get_ptr())) {
            ptr->i = -ptr->i;
            return true;
        }
        else if (auto ptr = dynamic_cast<double_wrapper* const>(n.content.get_ptr())) {
            ptr->d = -ptr->d;
            return true;
        }
        return false;
    }
};

struct sub_reduce : conv_base {
    using conv_base::conv_base;
    post_order_iterator apply_on(post_order_iterator it) override {
        double arg = 0;
        std::vector<post_order_iterator> give_up;
        for (unsigned i = 1; i != it->args.size();) {
            auto t = it.get_at(i);
            if (is_plain(t)) {
                arg += tree.cast_to_double(it.get_at(i), {});
                tree.erase(it.get_at(i));
            }
            else if (is_add(t)) {
                transfer_children(it.get_at(i), it);
                tree.erase(it.get_at(i));
            }
            else if (is_minus(t)) {
                if (transfer_from_minus(it.get_at(i), it))
                    tree.erase(it.get_at(i));
                else {
                    give_up.push_back(it.get_at(i));
                    ++i;
                }
            }
            else {
                give_up.push_back(it.get_at(i));
                ++i;
            }
        }
        combine_like_terms(tree).apply_on(give_up);
        {
            auto t = it.get_at(0);
            if (is_plain(t)) {
                t->content = arg_t(tree.cast_to_double(t, {}) - arg);
                arg = 0;
            }
            else if (is_add(t)) {
                auto itptr = tree.detach(it);
                auto tptr = tree.detach(t);
                t->parent = it->parent;
                if (!t->parent)
                    tree.reset(std::move(tptr));
                else
                    t->parent->args.insert(t->parent->args.begin(), std::move(tptr));
                it->parent = &*t;
                t->args.push_back(std::move(itptr));
                tree.insert_child_at(it, 0, arg_t(0));
            }
            else if (is_minus(t)) {
                transfer_children_first(it);
            }
            else {
                combine_like_terms_minus_first(tree).apply_on(it);
            }
        }
        if (it->args.empty())
            it->content = arg_t(arg);
        else if (it->args.size() == 1 && double_equal_abs_only(arg, 0))
            it = tree.replace_existing_node(it, it.get_at(0));
        else if (!double_equal_abs_only(arg, 0) || it->args.size() == 1)
            tree.insert_as_child(it, arg_t(arg));
        return ++it;
    }
private:
    void transfer_children(post_order_iterator from, post_order_iterator to) {
        to->args.reserve(from->args.size() + to->args.size());
        tree.move_children(to, from->args.begin(), from->args.end());
    }
    void transfer_children_first(post_order_iterator it) {
        it->args.reserve(it->args.size() + it->args[0]->args.size() - 1);
        tree.move_children(it, it->args.begin() + 1, it->args.end());
        tree.replace_existing_node(it, it.get_at(0));
    }
    bool transfer_from_minus(post_order_iterator from, post_order_iterator to) {
        to->args.reserve(from->args.size() + to->args.size() - 1);
        bool all_transferred = true;
        tree.move_child(from.get_at(0), to);
        for (unsigned i = 0; i != from->args.size();) {
            if (negate(*from.get_at(i)))
                tree.move_child(from.get_at(i), to);
            else {
                all_transferred = false;
                ++i;
            }
        }
        if (!all_transferred || from->args.size() == 1)
            tree.insert_child_at(from, 0, arg_t(0));
        return all_transferred;
    }
    static bool negate(node_t& n) {
        if (auto ptr = dynamic_cast<int_wrapper* const>(n.content.get_ptr())) {
            ptr->i = -ptr->i;
            return true;
        }
        else if (auto ptr = dynamic_cast<double_wrapper* const>(n.content.get_ptr())) {
            ptr->d = -ptr->d;
            return true;
        }
        return false;
    }
};

struct mul_reduce : conv_base {
    using conv_base::conv_base;
    post_order_iterator apply_on(post_order_iterator it) override {
        double arg = 1;
        for (unsigned i = 0; i != it->args.size();) {
            auto t = it.get_at(i);
            if (is_plain(t)) {
                arg *= tree.cast_to_double(it.get_at(i), {});
                tree.erase(it.get_at(i));
            }
            else if (is_mul(t)) {
                transfer_children(it.get_at(i), it);
                tree.erase(it.get_at(i));
            }
            else if (is_div(t)) {
                if (transfer_from_div(it.get_at(i), it))
                    tree.erase(it.get_at(i));
                else
                    ++i;
            }
            else {
                ++i;
            }
        }
        if (it->args.empty() || double_equal_abs_only(arg, 0))
            it->content = arg_t(arg);
        else if (it->args.size() == 1 && double_equal_abs_only(arg, 1))
            it = tree.replace_existing_node(it, it.get_at(0));
        else if (!double_equal_abs_only(arg, 1) || it->args.size() == 1)
            tree.insert_as_child(it, arg_t(arg));
        // RESET FLAGS AND RELEASE MEMORY
        return ++it;
    }
private:
    void transfer_children(post_order_iterator from, post_order_iterator to) {
        to->args.reserve(from->args.size() + to->args.size());
        tree.move_children(to, from->args.begin(), from->args.end());
    }
    bool transfer_from_div(post_order_iterator from, post_order_iterator to) {
        to->args.reserve(from->args.size() + to->args.size() - 1);
        bool all_transferred = true;
        tree.move_child(from.get_at(0), to);
        for (unsigned i = 0; i != from->args.size();) {
            if (reciprocate(*from.get_at(i)))
                tree.move_child(from.get_at(i), to);
            else {
                all_transferred = false;
                ++i;
            }
        }
        if (!all_transferred || from->args.size() == 1)
            tree.insert_child_at(from, 0, arg_t(1));
        return all_transferred;
    }
    static bool reciprocate(node_t& n) {
        if (auto ptr = dynamic_cast<int_wrapper* const>(n.content.get_ptr())) {
            ptr->i = 1 / ptr->i;
            return true;
        }
        else if (auto ptr = dynamic_cast<double_wrapper* const>(n.content.get_ptr())) {
            ptr->d = 1 / ptr->d;
            return true;
        }
        return false;
    }
};

struct div_reduce : conv_base {
    using conv_base::conv_base;
    post_order_iterator apply_on(post_order_iterator it) override {
        double arg = 1;
        for (unsigned i = 1; i != it->args.size();) {
            auto t = it.get_at(i);
            if (is_plain(t)) {
                arg *= tree.cast_to_double(it.get_at(i), {});
                tree.erase(it.get_at(i));
            }
            else if (is_mul(t)) {
                transfer_children(it.get_at(i), it);
                tree.erase(it.get_at(i));
            }
            else if (is_div(t)) {
                if (transfer_from_div(it.get_at(i), it))
                    tree.erase(it.get_at(i));
                else
                    ++i;
            }
            else
                ++i;
        }
        {
            auto t = it.get_at(0);
            if (is_plain(t)) {
                t->content = arg_t(tree.cast_to_double(t, {}) / arg);
                arg = 1;
            }
            else if (is_mul(t)) {
                auto itptr = tree.detach(it);
                auto tptr = tree.detach(t);
                t->parent = it->parent;
                if (!t->parent)
                    tree.reset(std::move(tptr));
                else
                    t->parent->args.push_back(std::move(tptr));
                it->parent = &*t;
                t->args.push_back(std::move(itptr));
                tree.insert_child_at(it, 0, arg_t(1));
            }
            else if (is_div(t)) {
                //delete it->args[0];
                transfer_children_first(it);
            }
            else
                ;
        }
        if (it->args.empty())
            it->content = arg_t(arg);
        else if (it->args.size() == 1 && double_equal_abs_only(arg, 1))
            it = tree.replace_existing_node(it, it.get_at(0));
        else if (!double_equal_abs_only(arg, 1) || it->args.size() == 1)
            tree.insert_as_child(it, arg_t(arg));
        return ++it;
    }
private:
    void transfer_children(post_order_iterator from, post_order_iterator to) {
        to->args.reserve(from->args.size() + to->args.size());
        tree.move_children(to, from->args.begin(), from->args.end());
    }
    void transfer_children_first(post_order_iterator it) {
        it->args.reserve(it->args.size() + it->args[0]->args.size() - 1);
        tree.move_children(it, it->args.begin() + 1, it->args.end());
        tree.replace_existing_node(it, it.get_at(0));
    }
    bool transfer_from_div(post_order_iterator from, post_order_iterator to) {
        to->args.reserve(from->args.size() + to->args.size() - 1);
        bool all_transferred = true;
        tree.move_child(from.get_at(0), to);
        for (unsigned i = 0; i != from->args.size();) {
            if (reciprocate(*from.get_at(i)))
                tree.move_child(from.get_at(i), to);
            else {
                all_transferred = false;
                ++i;
            }
        }
        if (!all_transferred || from->args.size() == 1)
            tree.insert_child_at(from, 0, arg_t(1));
        return all_transferred;
    }
    static bool reciprocate(node_t& n) {
        if (auto ptr = dynamic_cast<int_wrapper* const>(n.content.get_ptr())) {
            ptr->i = 1 / ptr->i;
            return true;
        }
        else if (auto ptr = dynamic_cast<double_wrapper* const>(n.content.get_ptr())) {
            ptr->d = 1 / ptr->d;
            return true;
        }
        return false;
    }
};

struct pow_reduce : conv_base {
    using conv_base::conv_base;
    post_order_iterator apply_on(post_order_iterator it) override {
        double arg = 1;
        for (unsigned i = 1; i != it->args.size();) {
            auto t = it.get_at(i);
            if (is_plain(t)) {
                arg *= tree.cast_to_double(it.get_at(i), {});
                tree.erase(it.get_at(i));
            }
            else {
                ++i;
            }
        }
        {
            auto t = it.get_at(0);
            if (is_plain(t)) {
                t->content = arg_t(pow(tree.cast_to_double(t, {}), arg));
                arg = 1;
            }
            else if (is_pow(t)) {
                it->args.reserve(it->args.size() + t->args.size() - 1);
                tree.move_children(it, it->args.begin() + 1, it->args.end());
                tree.replace_existing_node(it, t);
            }
        }
        if (double_equal_abs_only(arg, 0))
            it->content = arg_t(1);
        else if (it->args.size() == 1 && double_equal_abs_only(arg, 1))
            it = tree.replace_existing_node(it, it.get_at(0));
        else if (!double_equal_abs_only(arg, 1) || it->args.size() == 1)
            tree.insert_as_child(it, arg_t(arg));
        // RESET FLAGS AND RELEASE MEMORY
        return ++it;
    }
};

void simplify(expression_tree& expr) {
    static const map<string, double> fake_empty_dict{};
    struct generator final : utils::vgen_base<double> {
        const expression_tree::node& n;
        decltype(n.args)::const_iterator it;
        generator(const expression_tree::node& n) : n(n), it(n.args.begin()) {
            operator++();
        }
        generator& operator++() {
            if (it != n.args.end()) {
                set((*it)->content.to_double(fake_empty_dict));
                ++it;
                start();
            }
            else {
                end();
            }
            return *this;
        }
    };
    for (auto it = expr.post_order_begin(); it != expr.post_order_end();) {
        expression_tree::node& node = *it;
        if (!node.args.empty()) {
            if (!node.var) { // non volatile
                generator g(node);
                node.content = maybe_ftoi(expr._get_op_func(node)(g));
                node.args.resize(0);
                ++it;
            }
            else {
                auto op = expression_tree::as_op(node.content);
                if (op == &operators::PLU_OP)
                    it = add_reduce(expr).apply_on(it);
                else if (op == &operators::MIN_OP)
                    it = sub_reduce(expr).apply_on(it);
                else if (op == &operators::MUL_OP)
                    it = mul_reduce(expr).apply_on(it);
                else if (op == &operators::DIV_OP)
                    it = div_reduce(expr).apply_on(it);
                else if (op == &operators::POW_OP)
                    it = pow_reduce(expr).apply_on(it);
                else
                    ++it;
            }
        }
        else ++it;
    }
}
}

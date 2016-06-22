#ifndef PHYCO_MATH_H
#define PHYCO_MATH_H
#include <vector>
#include <memory>
#include <numeric>
#if defined(_DEBUG) || defined(PHYCO_DEBUG)
#include <iostream>
#endif
#include "phycoparse.h"
#include "operators.hpp"
#include "math_fwd.hpp"
#include "detail/phyco_mem.hpp"

namespace math {

//! Basic class for all mathematical exppressions. This is implemented with a 
//! tree with unknown numbers of children for each node.
class expression_tree {
public:
    struct node {
        friend expression_tree;
        arg_t content;
        node* parent;
        std::vector<node*> args;
        node(arg_t c, node* parent) :
            content(std::move(c)), parent(parent) {}
    private:
        //! Non-standard copy ctor for node.
        node(const node& other, node* parent) :content(copy_arg(other.content)), parent(parent) {
            args.reserve(other.args.size());
            for (auto i : other.args)
                append_copy(i);
        }
        struct copy_arg_visitor :utils::variant::static_visitor<arg_t> {
            template <class T>
            arg_t operator()(T a) const {
                return arg_t(a);
            }
        };
        static arg_t copy_arg(const arg_t& a) {
            return utils::variant::apply_visitor(copy_arg_visitor(), a);
        }
    public:
        ~node() noexcept {
            for (auto i : args)
                utils::allocator<node>::deallocate(i);
        }
        //! Get depth from root.
        unsigned depth() const noexcept {
            unsigned i = 0;
            auto temp = this;
            while (temp = temp->parent)
                ++i;
            return i;
        }
        //! Append a new `arg_t`.
        void append(arg_t arg) {
            args.push_back(new(utils::allocator<node>::allocate(1)) node(std::move(arg), this));
        }
        //! Append an existing node. This doesn't relink the relation.
        void append(node* arg) {
            args.push_back(arg);
        }
        //! Copy the content of an existing node, and append the copy.
        void append_copy(node* other) {
            args.push_back(new(utils::allocator<node>::allocate(1)) node(*other, this));
        }
        //! Erase a node from `args`, return the next iterator.
        auto erase(decltype(args)::iterator i) noexcept {
            utils::allocator<node>::deallocate(*i);
            return args.erase(i);
        }
    };
private:
    static decltype(node::args) __dummy_vector;
private:
    template<class T>
    class __post_order_iterator {
    public:
        typedef typename std::remove_pointer<T>::type value_type;
        typedef T pointer;
        typedef value_type& reference;
        typedef std::bidirectional_iterator_tag iterator_category;
        pointer ptr;
        decltype(node::args)::iterator it;
    public:
        explicit __post_order_iterator(node* ptr) : ptr(ptr) {
            if (ptr->parent)
                it = std::find(ptr->parent->args.begin(), ptr->parent->args.end(), ptr);
            else
                it = __dummy_vector.begin();
        }
        static __post_order_iterator make_null() noexcept {
            __post_order_iterator temp;
            temp.ptr = nullptr;
            return temp;
        }
    private:
        __post_order_iterator() {}
    public:
        reference operator*() const noexcept {
            return *ptr;
        }
        pointer operator->() noexcept {
            return ptr;
        }
        __post_order_iterator& operator++() noexcept {
            if (ptr->parent) {
                ++it;
                if (it != ptr->parent->args.end()) {
                    while (!(*it)->args.empty())
                        it = (*it)->args.begin();
                    ptr = *it;
                }
                else {
                    ptr = ptr->parent;
                    it = ptr->parent ? std::find(ptr->parent->args.begin(), ptr->parent->args.end(), ptr) : __dummy_vector.begin();
                }
            }
            else {
                ptr = ptr->parent;
            }
            return *this;
        }
        bool operator==(const __post_order_iterator& rhs) const noexcept {
            return ptr == rhs.ptr;
        }
        bool operator!=(const __post_order_iterator& rhs) const noexcept {
            return ptr != rhs.ptr;
        }
    };
    node* mroot;
public:
    typedef __post_order_iterator<node*> post_order_iterator;
    typedef __post_order_iterator<const node*> const_post_order_iterator;
    //! Default ctor for expression_tree, initialize the tree with 0+.
    expression_tree() {
        mroot = new(utils::allocator<node>::allocate(1)) node(0, nullptr);
    }
private:
    friend eqn_t;
    friend parse_manager;
    expression_tree(const expression_tree& other) {
        mroot = new(utils::allocator<node>::allocate(1)) node(*other.mroot, nullptr);
    }
    expression_tree& operator=(const expression_tree&) = delete;
public:
    expression_tree(expression_tree&& other) noexcept : mroot(other.mroot) {
        other.mroot = nullptr;
    }
    expression_tree& operator=(expression_tree&& other) noexcept {
        this->~expression_tree();
        mroot = other.mroot;
        other.mroot = nullptr;
        return *this;
    }
    ~expression_tree() noexcept {
        utils::allocator<node>::deallocate(mroot);
    }
public:
    static double cast_to_double(const node* mroot, const std::unordered_map<string, double>& vardict) {
        return utils::variant::apply_visitor(cast_to_double_visitor(mroot, vardict), mroot->content);
    }
    //! Execute the expression at certain node, return an int if possible, else double.
    static arg_t exec_at(node* mroot, const std::unordered_map<string, double>& vardict = {}) {
        return ftoi(utils::variant::apply_visitor(cast_to_double_visitor(mroot, vardict), mroot->content));
    }
private:
    //! Try to convert a double to int, store the result in arg_t.
    static arg_t ftoi(double x) noexcept {
        double i = 0;
        if (!std::modf(x, &i))
            return (int)i;
        else
            return x;
    }
    static double atof(arg_t a) {
        if (a.tag == 0)
            return a.i;
        else if (a.tag == 1)
            return a.d;
        throw;
    }
    struct cast_to_double_visitor :utils::variant::static_visitor<double> {
        const node* root;
        const std::unordered_map<string, double>& vardict;
        cast_to_double_visitor(const node* root, const std::unordered_map<string, double>& vardict) : root(root), vardict(vardict) {}
        double operator()(int a) const {
            return a;
        }
        double operator()(double a) const {
            return a;
        }
        double operator()(string a) const {
            return vardict.at(a);
        }
        double operator()(const Op* op) const {
            return reduce(root, vardict);
        }
    };
public:
    //! Execute the whole expression, return a double.
    double fexec(const std::unordered_map<string, double>& vardict = {}) const {
        return cast_to_double(mroot, vardict);
    }
    post_order_iterator post_order_begin() const noexcept {
        node* start = mroot;
        while (!start->args.empty())
            start = *start->args.begin();
        return post_order_iterator(start);
    }
    const_post_order_iterator const_post_order_begin() const noexcept {
        node* start = mroot;
        while (!start->args.empty())
            start = *start->args.begin();
        return const_post_order_iterator(start);
    }
    static post_order_iterator post_order_end() noexcept {
        return post_order_iterator::make_null();
    }
    static const_post_order_iterator const_post_order_end() noexcept {
        return const_post_order_iterator::make_null();
    }
    //! Execute at some operator node.
    static double reduce(const node* ptr, const std::unordered_map<string, double>& vardict) {
        double arg = cast_to_double(ptr->args[0], vardict);
        for (auto it = ++ptr->args.begin(); it != ptr->args.end(); ++it)
            arg = ptr->content.o->func(arg, cast_to_double(*it, vardict));
        if (!ptr->content.o->infix())
            arg = ptr->content.o->func(arg, 0);
        return arg;
    }
private:
    struct str_visitor :utils::variant::static_visitor<string> {
        const node* root;
        str_visitor(const node* root) : root(root) {}
        string operator()(int a) const {
            return (a < 0) ? "(" + std::to_string(a) + ")" : std::to_string(a);
        }
        string operator()(double a) const {
            return a < 0 ? "(" + std::to_string(a) + ")" : std::to_string(a);
        }
        string operator()(string a) const {
            return a;
        }
        string operator()(const Op* op) const {
            return expand(root, op->name);
        }
    };
public:
    //! Convert a sub-tree to string.
    static string str_at(const node* ptr) noexcept {
        return utils::variant::apply_visitor(str_visitor(ptr), ptr->content);
    }
    //! Expand to string at some operator node. Output infix expression.
    static string expand(const node* ptr, string name) noexcept {
        string str = "(" + str_at(ptr->args[0]) + name;
        for (auto it = ++ptr->args.begin(); it != ptr->args.end(); ++it) {
            str += str_at(*it) + name;
        }
        str = str.substr(0, str.size() - name.size()) + ")";
        return str;
    }
    // Convert the whole tree to string.
    string repr() const noexcept {
        return mroot ? str_at(mroot) : "";
    }
};

expression_tree parse(string, trie&);
expression_tree eval(const expression_tree&, std::unordered_map<string, double>);

class eqn_t {
private:
    expression_tree lhs, rhs;
public:
    eqn_t(const expression_tree&, const expression_tree&);
    //eqn_t(const eqn_t&) = delete;
    operator bool() const;
    void group_to_left();
    arg_t swap() noexcept;
    double fNsolve(string varname = "x", double seed = 0.01);
    inline string repr() const noexcept {
        return lhs.repr() + "=" + rhs.repr();
    }
};

#include "simplify.hpp"
#include "calculus.hpp"
#include "solver.hpp"
}
#ifndef PY_EXT
std::ostream& operator<<(std::ostream&, const math::arg_t&);
std::ostream& operator<<(std::ostream&, const math::expression_tree::node&);
std::ostream& operator<<(std::ostream&, const math::eqn_t&);
#endif
#endif

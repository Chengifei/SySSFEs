#ifndef EXPRESSION_TREE_HPP
#define EXPRESSION_TREE_HPP
#include <vector>
#include <algorithm>
#include <memory>
#include <type_traits>
#include "arg_t.hpp"
#include "simplify.hpp"
#include "detail/phyco_mem.hpp"

namespace math {

class expression_tree {
public:
    struct node {
        friend expression_tree;
        arg_t content;
        node* parent;
        bool var = false;
        std::vector<std::unique_ptr<node>> args;
        node(arg_t c, node* const parent) :
            content(std::move(c)), parent(parent) {
        }
        node(const node& other) :
            content(other.content), parent(other.parent) {
            args.reserve(other.args.size());
            for (const auto& nodeptr : other.args)
                args.push_back(std::make_unique<node>(*nodeptr));
        }
        node(node&&) noexcept = default;
        node& operator=(node&& other) noexcept = default;
    public:
        ~node() = default;
    };
private:
    template<class T>
    class _post_order_iterator {
    public:
        typedef typename std::remove_pointer<T>::type value_type;
        typedef T pointer;
        typedef value_type& reference;
        typedef std::forward_iterator_tag iterator_category;
    private:
        friend class expression_tree;
        pointer ptr;
    public:
        explicit _post_order_iterator(pointer const ptr) : ptr(ptr) {}
    public:
        reference operator*() const noexcept {
            return *ptr;
        }
        pointer operator->() noexcept {
            return ptr;
        }
        _post_order_iterator& operator++() noexcept {
            if (ptr->parent) {
                auto vit = get_vit();
                if (++vit < ptr->parent->args.end()) {
                    ptr = vit->get();
                    while (!ptr->args.empty()) {
                        ptr = ptr->args.front().get();
                    }
                }
                else ptr = ptr->parent;
            }
            else ptr = ptr->parent;
            return *this;
        }
        unsigned get_idx() const noexcept {
            for (unsigned i = 0; i != ptr->parent->args.size(); ++i)
                if (ptr->parent->args[i].get() == ptr)
                    return i;
            return -1;
        }
        auto get_vit() const noexcept {
            return std::find_if(ptr->parent->args.begin(), ptr->parent->args.end(),
                [this](const typename decltype(value_type::args)::value_type& u){ return u.get() == ptr; });
        }
        _post_order_iterator get_at(unsigned idx) const noexcept {
            return _post_order_iterator(ptr->args[idx].get());
        }
        bool operator==(const _post_order_iterator& rhs) const noexcept {
            return ptr == rhs.ptr;
        }
        bool operator!=(const _post_order_iterator& rhs) const noexcept {
            return ptr != rhs.ptr;
        }
        operator _post_order_iterator
            <typename std::add_pointer<
            typename std::add_const<value_type>::type>::type>() const noexcept {
            return _post_order_iterator<typename std::add_pointer<
                typename std::add_const<value_type>::type>::type>(ptr);
        }
    };
    template <class T>
    class _pre_order_iterator {
    public:
        typedef typename std::add_const<T>::type value_type;
        typedef typename std::add_pointer<value_type>::type pointer;
        typedef value_type& reference;
        typedef std::forward_iterator_tag iterator_category;
    private:
        pointer ptr;
    public:
        explicit _pre_order_iterator(pointer const ptr) : ptr(ptr) {}
    public:
        reference operator*() const noexcept {
            return *ptr;
        }
        pointer operator->() noexcept {
            return ptr;
        }
        _pre_order_iterator& operator++() noexcept {
            if (!ptr->args.empty())
                ptr = ptr->args[0].get();
            else {
                do {
                    if (ptr->parent) {
                        auto idx = get_idx();
                        if (++idx < ptr->parent->args.size()) {
                            ptr = ptr->parent->args[idx].get();
                            break;
                        }
                    }
                } while ((ptr = ptr->parent));
            }
            return *this;
        }
        unsigned get_idx() const noexcept {
            for (unsigned i = 0; i != ptr->parent->args.size(); ++i)
                if (ptr->parent->args[i].get() == ptr)
                    return i;
            return -1;
        }
        bool operator==(const _pre_order_iterator& rhs) const noexcept {
            return ptr == rhs.ptr;
        }
        bool operator!=(const _pre_order_iterator& rhs) const noexcept {
            return ptr != rhs.ptr;
        }
    };
    struct node_slot {
        node_slot* parent;
        const node& n;
        std::vector<std::unique_ptr<node_slot>> args;
        double field;
        bool available = false;
        node_slot(node_slot* parent, const node& n) :
            parent(parent), n(n) {}
        node_slot(const node_slot&) = delete;
        node_slot& operator=(const node_slot&) = delete;
    };
    struct filler;
private:
    std::unique_ptr<node> mroot = nullptr;
    mutable bool slot_available = false;
    static thread_local map<const expression_tree*, std::unique_ptr<node_slot>> slot_map;
    friend class eqn_t;
    friend class parse_manager;
    friend void simplify::simplify(expression_tree&);
    friend expression_tree& eval(expression_tree&, const map<string, double>&);
public:
    typedef _post_order_iterator<node*> post_order_iterator;
    typedef _post_order_iterator<const node*> const_post_order_iterator;
    typedef _post_order_iterator<node_slot*> slot_post_order_iterator;
    typedef _pre_order_iterator<node> pre_order_iterator;
    expression_tree() {
        mroot = std::make_unique<node>(arg_t(0), nullptr);
    }
    expression_tree(const expression_tree& other) = delete;
    expression_tree& operator=(const expression_tree&) = delete;
    expression_tree(expression_tree&& other) noexcept : mroot(std::move(other.mroot)) {}
    expression_tree& operator=(expression_tree&& other) noexcept {
        mroot = std::move(other.mroot);
        return *this;
    }
    ~expression_tree() noexcept = default;
    // ITERATOR SUPPORT
private:
    post_order_iterator _post_order_root() const noexcept {
        return post_order_iterator(mroot.get());
    }
    pre_order_iterator pre_order_begin() const noexcept {
        return pre_order_iterator(mroot.get());
    }
public:
    post_order_iterator post_order_begin() const noexcept {
        node* start = mroot.get();
        while (!start->args.empty())
            start = start->args.cbegin()->get();
        return post_order_iterator(start);
    }
    static post_order_iterator post_order_end() noexcept {
        return post_order_iterator(nullptr);
    }
    const_post_order_iterator const_post_order_begin() const noexcept {
        const node* start = mroot.get();
        while (!start->args.empty())
            start = start->args.cbegin()->get();
        return const_post_order_iterator(start);
    }
    static const_post_order_iterator const_post_order_end() noexcept {
        return const_post_order_iterator(nullptr);
    }
    static slot_post_order_iterator slot_post_order_begin(node_slot* ptr) noexcept {
        node_slot* start = ptr;
        while (!start->args.empty())
            start = start->args.cbegin()->get();
        return slot_post_order_iterator(start);
    }
private:
    template <class T>
    struct _iter_type {
    private:
        T _begin, _end;
    public:
        _iter_type(T begin, T end) : _begin(begin), _end(end) {}
        T begin() const noexcept {
            return _begin;
        }
        T end() const noexcept {
            return _end;
        }
    };
public:
    _iter_type<post_order_iterator> iter() noexcept {
        return{ post_order_begin(), post_order_end() };
    }
    _iter_type<const_post_order_iterator> citer() const noexcept {
        return{ const_post_order_begin(), const_post_order_end() };
    }
public:
    static double cast_to_double(post_order_iterator mroot, const map<string, double>& vardict) {
        return mroot->content.to_double(vardict);
    }
private:
    static const Op* const as_op(const arg_t& arg) noexcept {
        return static_cast<const op_wrapper* const>(arg.get_ptr())->get_ptr();
    }
    static auto& _get_op_func(const node& ptr) {
        return *as_op(ptr.content);
    }
    std::unique_ptr<node_slot> make_slots() const;
public:
    //! Execute the whole expression, return a double.
    double fexec(const map<string, double>& vardict = {}) const;
#ifdef PY_EXT
    double fexec(const boost::python::dict&) const;
#endif
public:
    post_order_iterator insert_as_child(post_order_iterator, arg_t&&);
    post_order_iterator insert_child_at(post_order_iterator, unsigned, arg_t&&);
    post_order_iterator move_child(post_order_iterator, post_order_iterator);
    post_order_iterator move_child_to(post_order_iterator, unsigned, post_order_iterator);
    template <class iterator>
    void move_children(post_order_iterator parent, iterator begin, iterator end) {
        for (; begin != end; ++begin) {
            (*begin)->parent = &*parent;
            parent->args.push_back(std::move(*begin));
        }
    }
    void move_tree(expression_tree&&, post_order_iterator);
    void reset(std::unique_ptr<node>&& ptr) noexcept;
    //post_order_iterator replace_existing_node(post_order_iterator, arg_t&&);
    post_order_iterator replace_existing_node(post_order_iterator, post_order_iterator) noexcept;
    post_order_iterator insert_as_parent(post_order_iterator, arg_t&&);
    post_order_iterator erase(post_order_iterator) noexcept;
    std::unique_ptr<node> detach(post_order_iterator) noexcept;
public:
    //! Expand to string at some operator node. Output infix expression.
    static string expand(const node*, string name) noexcept;
    // Convert the whole tree to string.
    string repr() const noexcept {
        if (mroot)
            if (mroot->args.empty())
                return mroot.get()->content.to_string();
            else
                return expand(mroot.get(), as_op(mroot->content)->name);
        else
            return "";
    }
};
}
#endif

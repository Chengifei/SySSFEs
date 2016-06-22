#ifndef PHYCO_DETAIL_QUAD_LIST_HPP
#define PHYCO_DETAIL_QUAD_LIST_HPP
#include <iterator>
#include <algorithm>
#include "../pool/pool.hpp"
#include "quad_node.hpp"
#include "../AVL/avl.hpp"

namespace utils {

class quad_list {
    const static unsigned split = 8;
    const static unsigned max = 104;
    const static unsigned min = 96;
    const static unsigned level_max = (max - min) / split + 2;
    typedef intervalp T;
private:
    class iterator {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef quad_node value_type;
        typedef quad_node* pointer;
        typedef quad_node& reference;
        typedef std::bidirectional_iterator_tag iterator_category;
    private:
        friend quad_list;
        pointer ptr;
    public:
        iterator(pointer ptr) : ptr(ptr) {}
        iterator(iterator&& it) noexcept : ptr(it.ptr) {}
        iterator& operator=(iterator&& other) {
            ptr = other.ptr;
            return *this;
        }
        iterator(const iterator&) = default;
        iterator& operator=(const iterator&) = default;
    public:
        quad_list::T* operator*() noexcept {
            return ptr->content;
        }
        iterator& operator++() noexcept {
            ptr = ptr->right;
            return *this;
        }
        iterator operator++(int) noexcept {
            auto tmp = ptr;
            ptr = ptr->right;
            return tmp;
        }
        iterator& operator--() noexcept {
            ptr = ptr->left;
            return *this;
        }
        iterator operator--(int) noexcept {
            auto tmp = ptr;
            ptr = ptr->left;
            return tmp;
        }
        bool operator==(const iterator& b) const noexcept {
            return ptr == b.ptr;
        }
        bool operator!=(const iterator& b) const noexcept {
            return ptr != b.ptr;
        }
        operator bool() const noexcept {
            return ptr != nullptr;
        }
        quad_list::T* operator->() const noexcept {
            return ptr->content;
        }
        iterator left() const noexcept {
            return iterator(ptr->left);
        }
        iterator right() const noexcept {
            return iterator(ptr->right);
        }
        iterator up() const noexcept {
            return iterator(ptr->up);
        }
        iterator down() const noexcept {
            return iterator(ptr->down);
        }
        iterator base() const noexcept {
            auto tmp = ptr;
            while (tmp->down)
                tmp = tmp->down;
            return iterator(tmp);
        }
        iterator top(std::size_t sz, unsigned level) const noexcept {
            auto tmp = ptr;
            // while (tmp=tmp->up){}
            auto levels = proc_size(ptr->content->size + sz) - (1 + level);
            for (unsigned i = 0; i != levels; ++i)
                tmp = tmp->up;
            return iterator(tmp);
        }
        iterator topf(std::size_t sz, unsigned level) const noexcept {
            auto tmp = ptr;
            auto levels = proc_size(sz) - (1 + level);
            for (unsigned i = 0; i != levels; ++i)
                tmp = tmp->up;
            return iterator(tmp);
        }
        inline void up(quad_node* v) const noexcept {
            ptr->up = v;
        }
        inline void down(quad_node* v) const noexcept {
            ptr->down = v;
        }
        inline unsigned top_level(std::size_t sz = 0) const noexcept {
            //unsigned ret = 1;
            //auto tmp = ptr;
            //while (tmp->up) {
            //    tmp = tmp->up;
            //    ++ret;
            //}
            //tmp = ptr;
            //while (tmp->down) {
            //    tmp = tmp->down;
            //    ++ret;
            //}
            //return ret;
            return proc_size(ptr->content->size + sz);
        }
        inline pointer get_ptr() const noexcept {
            return ptr;
        }
    private:
        static inline unsigned get_level(std::size_t size) noexcept {
            if (size < min)
                return 0;
            if (size >= max)
                return level_max - 1;
            return (size - min) / split + 1;
        }
    };
private:
    friend sBST;
    sBST tree;
    quad_node* My_sentinels[level_max];
public:
    quad_list() {
        for (unsigned i = 0; i != level_max; ++i) {
            My_sentinels[i] = new(quad_node::buffer.allocate()) quad_node();
        }
        //for (unsigned i = 1; i != level_max; ++i) {
        //    My_sentinels[i]->down = My_sentinels[i - 1];
        //    My_sentinels[i - 1]->up = My_sentinels[i];
        //}
    }
    quad_list(const quad_list&) = delete;
private:
    static inline quad_node* get_from_buffer(T* value, quad_node* L, quad_node* R) {
        return new(quad_node::buffer.allocate()) quad_node(value, L, R);
    }
    static inline void return_to_buffer(quad_node* ptr) {
        quad_node::buffer.deallocate(ptr);
    }
public:
    inline iterator begin(unsigned level = 0) const noexcept {
        return iterator(My_sentinels[level]).right();
    }
    inline iterator end(unsigned level = 0) const noexcept {
        return iterator(My_sentinels[level]);
    }
public:
    void push(T); // push data in to structure
    uintptr_t pop(std::size_t, unsigned); // pop data out of structure
private:
    static inline unsigned proc_size(std::size_t sz) noexcept {
        if (sz < min)
            return 1;
        if (sz >= max)
            return level_max;
        return (sz - min) / split + 2; // force round up
    }
    iterator erase(iterator);
    inline static bool adjacent_check(T L, T R) noexcept {
        return L.low + L.size == R.low;
    }
    inline bool Ladjacent_check(iterator it) noexcept {
        return (it != begin()) ? it->low == it.left()->low + it.left()->size : false;
    }
    inline bool Radjacent_check(iterator it) noexcept {
        return (it.right() != end()) ? it->low + it->size == it.right()->low : false;
    }
    inline std::size_t merge(iterator&) noexcept;
    inline void publish(iterator, unsigned hint = 1) const;
public:
    inline bool empty(unsigned level = 0) const noexcept {
        return begin(level) == end(level);
    }
public:
    inline void clear() noexcept {
        tree.erase_node(tree.root);
    }
private:
    struct idx_range {
    private:
        iterator a, b;
    public:
        idx_range(iterator a, iterator b) :a(a), b(b) {};
        iterator begin() { return a; };
        iterator end() { return b; }
    };
public:
    idx_range operator[](unsigned i) const noexcept {
        return idx_range(begin(i), end(i));
    }
    ~quad_list() {
        for (auto i : My_sentinels)
            return_to_buffer(i);
    }
};

}

#endif

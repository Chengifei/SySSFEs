#ifndef PHYCO_DETAIL_AVL_HPP
#define PHYCO_DETAIL_AVL_HPP

#include "avl_node.hpp"
#include "../quad_list/quad_node.hpp"
#include "../pool/pool.hpp"
#ifdef PHYCO_DEBUG
#include <vector>
#endif

namespace utils {

class quad_list;

class sBST {
private:
    typedef intervalp T;
private:
    friend quad_list;
    avl_node* root = nullptr;
public:
    avl_node* search(T value) const noexcept {
        if (root) return search(root, value);
        else return nullptr;
    }
    avl_node* push(T, avl_node* pos = nullptr);
    void erase(T) noexcept;
    static avl_node* find_first_greater(avl_node* pos);
private:
    static inline avl_node* get_new_node(T value, avl_node* ptr) {
        return new(avl_node::buffer.allocate()) avl_node(value, ptr);
    }
    static inline void destroy_node(avl_node* ptr) noexcept {
        avl_node::buffer.deallocate(ptr);
    }
    void erase_node(avl_node*) noexcept;
    static inline avl_node* search(avl_node* pos, T value) noexcept {
        while (true) {
            if (value.low < pos->content.low)
                if (pos->Lchild) pos = pos->Lchild;
                else return pos;
            else if (value.low == pos->content.low)
                return pos;
            else
                if (pos->Rchild) pos = pos->Rchild;
                else return pos;
        }
    }
    static inline avl_node* find_left_subtree_rightmost(avl_node* pos) noexcept {
        pos = pos->Lchild;
        while (pos->Rchild)
            pos = pos->Rchild;
        return pos;
    }
    void unwind_and_balance_when_need(avl_node*, int tag = 1, bool from = false) noexcept;
    avl_node* balance(avl_node*) noexcept;
    inline avl_node* push_at(avl_node*, T) noexcept;
private: //Rotations
    inline avl_node* LR(avl_node*) noexcept;
    inline avl_node* RR(avl_node*) noexcept;
    inline avl_node* LRR(avl_node*) noexcept;
    inline avl_node* RLR(avl_node*) noexcept;
public:
    bool empty() const noexcept {
        return root == nullptr;
    }
public:
    ~sBST() {
        avl_node::buffer.deallocate(root);
    }
};

}

#endif

#ifndef PHYCO_DETAIL_AVL_NODE_HPP
#define PHYCO_DETAIL_AVL_NODE_HPP

#include "../interval/interval.hpp"
#include "../quad_list/quad_node.hpp"
#include "../pool/pool.hpp"

namespace utils {

struct avl_node {
private:
    typedef intervalp T;
public:
    static pool<avl_node, 1000> buffer;
public:
    T content;
    avl_node* parent;
    avl_node* Lchild = nullptr, *Rchild = nullptr;
    quad_node* up; //corresponding list node
    avl_node(const avl_node&) = delete;
    avl_node& operator=(const avl_node&) = delete;
    int tag = 0;
    inline bool LorR() const noexcept {
        // return 0 for left 1 for right
        // should have been improved by returning a bool enum
        if (parent)
            return content.low > parent->content.low;
        return 1; //unused
    }
    avl_node(T content, avl_node* parent) :content(content), parent(parent) {}
    avl_node() :content(0, 0) {}
    inline bool check_balance() const noexcept {
        auto a = get_R_tag() - get_L_tag();
        return -1 <= a && a <= 1;
    }
    inline int get_balance() const noexcept {
        // R-L
        return get_R_tag() - get_L_tag();
    }
private:
    inline int get_L_tag() const noexcept {
        return Lchild ? Lchild->tag : 0;
    }
    inline int get_R_tag() const noexcept {
        return Rchild ? Rchild->tag : 0;
    }
public:
    ~avl_node() noexcept {
        // be sure to release ownership before destruct
        // deleting both children prevents memory leak
        //delete Lchild;
        //delete Rchild;
        buffer.deallocate(Lchild);
        buffer.deallocate(Rchild);
        //delete up;
        quad_node::buffer.deallocate(up);
    }
};

}

#endif

#ifndef PHYCO_DETAIL_QUAD_NODE_HPP
#define PHYCO_DETAIL_QUAD_NODE_HPP
#include "../interval/interval.hpp"
#include "../pool/pool.hpp"
namespace utils {

struct avl_node;
class quad_list;

struct quad_node {
private:
    friend avl_node;
    friend quad_list;
    static pool<quad_node, 10000> buffer;
    typedef intervalp T;
    struct variant {
        union {
            quad_node* qn;
            avl_node* tn = nullptr;
        };
        enum :bool { TO_TREE = 0, TO_LIST = 1 } tag = TO_TREE;
        inline quad_node* operator->() const noexcept {
            if (tag)
                return qn;
            return nullptr;
        }
        inline operator bool() const noexcept {
            if (tag)
                return qn != nullptr;
            return false;
        }
        inline operator quad_node*() const noexcept {
            return (tag) ? qn : nullptr;
        }
        variant& operator=(quad_node* other) {
            qn = other;
            tag = variant::TO_LIST;
            return *this;
        }
        variant& operator=(avl_node* other) {
            tn = other;
            tag = variant::TO_TREE;
            return *this;
        }
    };
public:
    T* content;
    quad_node* up = nullptr, *left, *right;
    variant down; // in-class initializer
    quad_node(T* content, quad_node* left, quad_node* right) :
        content(content), left(left), right(right) {
        left->right = this;
        right->left = this;
    }
    quad_node() :content(nullptr), left(this), right(this) {}
    quad_node(const quad_node& other) = delete;
    quad_node& operator=(const quad_node& other) = delete;
    ~quad_node() noexcept;
};

}
#endif

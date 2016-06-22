#include "avl.hpp"
#ifdef PHYCO_DEBUG
#include <cassert>
#include <cstdio>
#endif
#ifdef PHYCO_DEBUG
#define printf(...) printf(__VA_ARGS__)
#else
#define printf(...)
#endif

namespace utils {

pool<avl_node, 1000> avl_node::buffer;

avl_node* sBST::push(T value, avl_node* pos) {
    if (!root) {
        root = get_new_node(value, nullptr);
        root->tag = 1;
        return root;
    }
    else {
        auto tmp = push_at(search((pos) ? pos : root, value), value);
        return tmp;
    }
}

avl_node* sBST::push_at(avl_node* pos, T value) noexcept {
    printf("push_at: 0x%p: %d %d\n", pos, value.low, value.size);
    if (value.low < pos->content.low) {
        //auto tmp = new avl_node(value, pos, false);
        auto tmp = get_new_node(value, pos);
        pos->Lchild = tmp;
        unwind_and_balance_when_need(tmp);
        return tmp;
    }
    else {
        //auto tmp = new avl_node(value, pos, true);
        auto tmp = get_new_node(value, pos);
        pos->Rchild = tmp;
        unwind_and_balance_when_need(tmp);
        return tmp;
    }
}

void sBST::erase(T value) noexcept {
    erase_node(search(root, value));
}

avl_node* sBST::find_first_greater(avl_node* pos) {
    while (pos->parent) {
        if (!pos->LorR()) // pos is Lchild
            return pos->parent;
        pos = pos->parent;
    }
    return nullptr; // fail
}

void sBST::erase_node(avl_node* pos) noexcept {
    printf("erase_node: 0x%p: %d\n", pos, pos->content.low);
    if (pos->Lchild && pos->Rchild) {
        auto const cand = find_left_subtree_rightmost(pos);
        auto P = pos->parent;
        auto A = pos->Lchild;
        pos->Lchild = nullptr;
        auto B = pos->Rchild;
        pos->Rchild = nullptr;
        if (A != cand) {
            // TODO: UNTESTED AREA
            avl_node* O; // cand's orginal parent;
            if (cand->Lchild) {
                cand->Lchild->parent = cand->parent;
                cand->parent->Rchild = cand->Lchild; // cand must be Rchild
                O = cand->Lchild;
            }
            else { // cand can't have Rchild, if it doesn't have Lchild, then it has no child
                cand->parent->Rchild = nullptr;
                O = cand->parent;
            }
            cand->parent = P;
            if (P)
                if (pos->LorR())
                    P->Rchild = cand;
                else P->Lchild = cand;
            else
                root = cand;
            cand->Lchild = A;
            A->parent = cand;
            cand->Rchild = B;
            B->parent = cand;
            //abort();
            unwind_and_balance_when_need(O, O->tag);
        }
        else { // cand is A
            cand->parent = P;
            if (P)
                if (pos->LorR()) P->Rchild = cand;
                else P->Lchild = cand;
            else
                root = cand;
            cand->Rchild = B;
            B->parent = cand;
            unwind_and_balance_when_need(B, B->tag);
        }
        destroy_node(pos);
    }
    else {
        if (pos->parent) {
            bool LR = pos->LorR();
            if (pos->Lchild) {
                pos->Lchild->parent = pos->parent;
                if (LR)
                    pos->parent->Rchild = pos->Lchild;
                else
                    pos->parent->Lchild = pos->Lchild;
                unwind_and_balance_when_need(pos->Lchild, pos->Lchild->tag);
                printf("1");
                //assert(!pos->Rchild);
                pos->Lchild = nullptr; // release ownership of child
            }
            else if (pos->Rchild) {
                pos->Rchild->parent = pos->parent;
                if (LR)
                    pos->parent->Rchild = pos->Rchild;
                else
                    pos->parent->Lchild = pos->Rchild;
                unwind_and_balance_when_need(pos->Rchild, pos->Rchild->tag);
                printf("2");
                pos->Rchild = nullptr;
            }
            else {
                if (LR) {
                    pos->parent->Rchild = nullptr;
                    bool from = (bool)pos->parent->Lchild;
                    unsigned tag = (from) ? pos->parent->Lchild->tag + 1 : 1;
                    unwind_and_balance_when_need(pos->parent, tag, !from);
                }
                else {
                    pos->parent->Lchild = nullptr;
                    bool from = (bool)pos->parent->Rchild;
                    unsigned tag = (from) ? pos->parent->Rchild->tag + 1 : 1;
                    unwind_and_balance_when_need(pos->parent, tag, from);
                }
                printf("3");
            }
            destroy_node(pos);
        }
        else {
            if (pos->Lchild) {
                root = pos->Lchild;
                root->parent = nullptr;
                pos->Lchild = nullptr; // release ownership
            }
            else if (pos->Rchild) {
                root = pos->Rchild;
                root->parent = nullptr;
                pos->Rchild = nullptr;
            }
            else {
                root = nullptr;
            }
            destroy_node(pos);
        }
    }
}

void sBST::unwind_and_balance_when_need(avl_node* pos, int tag, bool from) noexcept {
    while (pos) {
#ifdef PHYCO_DEBUG
        std::vector<avl_node*> vec;
        std::vector<bool> vec1;
        vec.push_back(pos);
        vec1.push_back(from);
#endif
        pos->tag = tag;
        if (!pos->check_balance()) {
            auto ret = balance(pos);
            unwind_and_balance_when_need(ret, ret->tag, from);
            return;
        }
        ++tag;
        //--------preparation for next cycle-----------
        from = pos->LorR();
        pos = pos->parent;
    }
}

avl_node* sBST::LR(avl_node* A) noexcept {
    printf("LR called 0x%p\n", A);
    printf("D:A %d\n", A->tag);
    A->tag -= 2;
    auto B = A->Rchild;
    B->parent = A->parent;
    if (A->parent)
        if (A->LorR())
            A->parent->Rchild = B;
        else
            A->parent->Lchild = B;
    else root = B;
    A->parent = B;
    A->Rchild = B->Lchild; // CAUTION!
    if (A->Rchild) A->Rchild->parent = A;
    B->Lchild = A;
    return B;
}

avl_node* sBST::RR(avl_node* A) noexcept {
    printf("RR called 0x%p\n", A);
    printf("D:A %d\n", A->tag);
    A->tag -= 2;
    auto B = A->Lchild;
    B->parent = A->parent;
    if (A->parent)
        if (A->LorR())
            A->parent->Rchild = B;
        else
            A->parent->Lchild = B;
    else root = B;
    A->parent = B;
    A->Lchild = B->Rchild;
    if (A->Lchild) A->Lchild->parent = A;
    B->Rchild = A;
    return B;
}

avl_node* sBST::LRR(avl_node* A) noexcept {
    printf("LRR called 0x%p\n", A);
    auto B = A->Lchild;
    auto C = B->Rchild;
    printf("B>1 in RLR:%d\n", B->tag > 1);
    --B->tag;
    ++C->tag;
    A->Lchild = C;
    C->parent = A;
    B->parent = C;
    C->Lchild = B;
    B->Rchild = nullptr;
    return RR(A);
}

avl_node* sBST::RLR(avl_node* A) noexcept {
    printf("RLR called 0x%p\n", A);
    auto B = A->Rchild;
    auto C = B->Lchild;
    printf("B>1 in RLR:%d\n", B->tag > 1);
    --B->tag;
    ++C->tag;
    A->Rchild = C;
    C->parent = A;
    B->parent = C;
    C->Rchild = B;
    B->Lchild = nullptr;
    return LR(A);
}

avl_node* sBST::balance(avl_node* A) noexcept {
    if (A->get_balance() < 0)
        if (A->Lchild->Lchild)
            if (A->Lchild->get_balance() <= 0)
                return RR(A);
            else {
                LR(A->Lchild);
                return RR(A);
            }
        else
            return LRR(A);
    else
        if (A->Rchild->Rchild)
            if (A->Rchild->get_balance() >= 0)
                return LR(A);
            else {
                RR(A->Rchild);
                return LR(A);
            }
        else
            return RLR(A);
}

}

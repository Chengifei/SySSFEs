#include "quad_list.hpp"
#ifdef PHYCO_DEBUG
#include <cassert>
#endif

namespace utils {

void quad_list::push(T value) {
#ifdef PHYCO_DEBUG
    printf("quad_list::push %d, %d\n", value.low, value.size);
#endif
    if (!empty()) {
        auto hint = tree.search(value);
        auto pos = iterator(hint->up);
        if (pos->low < value.low)
            ++pos;
        unsigned tmp;
        iterator top(nullptr);
        if (pos != begin() && adjacent_check(**(pos.left()), value)) {
            top = (--pos).top(0, 0);
            tmp = top.top_level();
            pos->Rmerge(value);
            //printf("1");
        }
        else if (pos != end() && adjacent_check(value, **pos)) {
            top = pos.top(0, 0);
            tmp = top.top_level();
            pos->Lmerge(value);
            //printf("2");
        }
        else {
            //auto in = new quad_node(new T(value), pos.get_ptr()->left, pos.get_ptr());
            auto tree_node = tree.push(value/*, hint*/);
            auto in = get_from_buffer(&(tree_node->content), pos.get_ptr()->left, pos.get_ptr());
            in->down = tree_node;
            tree_node->up = in;
            top = --pos;
            tmp = 1;
        }
        if (auto a = merge(pos)) { //in case of X-X when - is returned double merge is required
            // merge is entitled to modify pos
            top = pos.topf(a, 0);
            tmp = proc_size(a);
            //printf("3");
        }
        publish(top, tmp);
    }
    else {
        auto tree_node = tree.push(value);
        // push-back
        My_sentinels[0]->left = get_from_buffer(&(tree_node->content), (--end()).get_ptr(), My_sentinels[0]);
        begin().get_ptr()->down = tree_node;
        tree_node->up = begin().get_ptr();
        publish(begin().get_ptr());
    }
}

uintptr_t quad_list::pop(std::size_t sz, unsigned align) {
    auto level = proc_size(sz) - 1;
    if (!empty(level)) {
        auto last_it = --end(level);
        auto tmp = last_it->low;
        auto offset = tmp % align;
        if (offset == 0) {
            if (last_it->size > sz) {
                last_it->low += sz;
                last_it->size -= sz;
                if (auto remove_level = last_it.top_level(sz) - proc_size(last_it->size)) {
                    auto top = last_it.top(sz, level).down().get_ptr();
                    for (unsigned i = 0; i != remove_level; i++, top = top->down)
                        return_to_buffer(top->up);
                }
                return tmp;
            }
            else if (last_it->size == sz) {
                erase(last_it);
                return tmp;
            }
        }
        else {
            if (last_it->size > sz + offset) {
                last_it->low += sz + offset;
                last_it->size -= sz - offset;
                auto top = last_it.top(sz, level).down().get_ptr();
                auto remove_level = last_it.top_level(sz) - proc_size(last_it->size);
                for (unsigned i = 0; i != remove_level; i++, top = top->down)
                    return_to_buffer(top->up);
                push(T(tmp, tmp + offset));
                return tmp + offset;
            }
            else if (last_it->size == sz + offset) {
                last_it->size = offset;
                auto top = last_it.top(sz, level).down().get_ptr();
                auto remove_level = last_it.top_level(sz) - proc_size(last_it->size);
                for (unsigned i = 0; i != remove_level; i++, top = top->down)
                    //delete top->up;
                    return_to_buffer(top->up);
                return tmp + offset;
            }
        }
    }
    return 0; // 0 means failure
}

std::size_t quad_list::merge(iterator& it) noexcept {
    if (Ladjacent_check(it)) {
        auto ret = it->size;
        it->Lmerge(**(it.left()));
        erase(it.left());
        return ret;
    }
    else if (Radjacent_check(it)) {
        /*it->Rmerge(**(it.right()));
        erase(it.right());*/
        ++it;
        auto ret = it->size;
        it->Lmerge(**(it.left()));
        erase(it.left());
        return ret;
    }
    return 0;
}

void quad_list::publish(iterator it, unsigned hint) const {
    auto level = proc_size(it->size);
    auto ptr = it.get_ptr();
    for (unsigned i = /*1*/hint; i != level; ++i, ptr = ptr->up) {
        //for (; hint != level; ++hint, ptr = ptr->up) {
            //if (!ptr->up) {
        ptr->up = get_from_buffer(*it, (--end(i)).get_ptr(), end(i).get_ptr());
        ptr->up->down = ptr;
        //}
    }
}

quad_list::iterator quad_list::erase(iterator it) {
    it = it.base();
    // erase the tree node will also erase list node
    // which then relink other nodes
    tree.erase_node(it.get_ptr()->down.tn);
    return it;
}

}

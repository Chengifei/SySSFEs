#include "expression_tree.hpp"
#include "gen.hpp"

namespace math {

std::unique_ptr<expression_tree::node_slot>
expression_tree::make_slots() const { // constructs slots for execution in post order
    auto it = pre_order_begin();
    auto root = std::make_unique<node_slot>(nullptr, *it);
    node_slot* at = root.get();
    do {
        if (it->parent == at->n.parent) {
            if (it->parent)
                at = at->parent->args[it.get_idx()].get();
        }
        else if (it->parent == &at->n)
            at = at->args[it.get_idx()].get();
        else {
            while (it->parent != &at->n)
                at = at->parent;
            at = at->args[it.get_idx()].get();
        }
        for (const auto& child : it->args)
            at->args.push_back(std::make_unique<node_slot>(at, *child));
        ++it;
    } while (&*it != nullptr);
    return root;
}

thread_local map<const expression_tree*, std::unique_ptr<expression_tree::node_slot>> expression_tree::slot_map{};

struct expression_tree::filler final : utils::vgen_base<double> {
    const node_slot& n;
    decltype(n.args)::const_iterator it;
    filler(const node_slot& n) : n(n), it(n.args.begin()) {
        operator++();
    }
    filler& operator++() {
        if (it != n.args.end()) {
            set((*it)->field);
            ++it;
            start();
        }
        else {
            end();
        }
        return *this;
    }
};

double expression_tree::fexec(const map<string, double>& vardict) const {
    node_slot* slots;
    if (slot_available)
        slots = slot_map[this].get();
    else {
        slot_map[this] = make_slots();
        slot_available = true;
        slots = slot_map[this].get();
    }
    for (auto it = slot_post_order_begin(slots); &*it != nullptr; ++it) {
        node_slot& i = *it;
        if (!i.available) { // cache not available
            if (!i.args.empty()) {
                filler g(i);
                i.field = _get_op_func(i.n)(g);
            }
            else {
                i.field = i.n.content.to_double(vardict);
            }
            if (!i.n.var)
                i.available = true;
        }
    }
    return slots->field;
}

#ifdef PY_EXT
double expression_tree::fexec(const boost::python::dict& vardict) const {
    node_slot* slots;
    if (slot_available)
        slots = slot_map[this].get();
    else {
        slot_map[this] = make_slots();
        slot_available = true;
        slots = slot_map[this].get();
    }
    for (auto it = slot_post_order_begin(slots); &*it != nullptr; ++it) {
        node_slot& i = *it;
        if (!i.available) { // cache not available
            if (!i.args.empty()) {
                filler g(i);
                i.field = _get_op_func(i.n)(g);
            }
            else {
                try {
                    i.field = i.n.content.to_double(vardict);
                } catch (...) {
                    PyErr_SetString(PyExc_NameError, i.n.content.to_string().c_str());
                    boost::python::throw_error_already_set();
                }
            }
            if (!i.n.var)
                i.available = true;
        }
    }
    return slots->field;
}
#endif

expression_tree::post_order_iterator // return iterator to the inserted node
expression_tree::insert_as_child(post_order_iterator parent, arg_t&& value) {
    parent->args.push_back(std::make_unique<node>(std::move(value), parent.ptr));
    return post_order_iterator(parent->args.back().get());
}

expression_tree::post_order_iterator // return iterator to the inserted node
expression_tree::insert_child_at(post_order_iterator parent, unsigned idx, arg_t&& val) {
    parent->args.insert(parent->args.begin() + idx, std::make_unique<node>(std::move(val), &*parent));
    return parent.get_at(idx);
}

expression_tree::post_order_iterator //
expression_tree::move_child(post_order_iterator it, post_order_iterator new_parent) {
    assert(it->parent);
    new_parent->args.push_back(detach(it));
    it->parent = &*new_parent;
    return ++it;
}

void expression_tree::move_tree(expression_tree&& tree, post_order_iterator new_parent) {
    auto it = tree._post_order_root();
    it->parent = &*new_parent;
    new_parent->args.push_back(std::move(tree.mroot));
}

void expression_tree::reset(std::unique_ptr<node>&& ptr) noexcept {
    mroot = std::move(ptr);
    mroot->parent = nullptr;
}

expression_tree::post_order_iterator // return iterator to the inserted node
expression_tree::insert_as_parent(post_order_iterator child, arg_t&& value) {
    if (child->parent) {
        auto child_pos = child.get_idx();
        child->parent->args[child_pos].release();
        child->parent = new node(std::move(value), child->parent);
        child->parent->args.push_back(std::unique_ptr<node>(&*child));
        child->parent->parent->args[child_pos] = std::unique_ptr<node>(child->parent);
        return post_order_iterator(child->parent);
    }
    else {
        mroot.release();
        mroot = std::make_unique<node>(std::move(value), nullptr);
        child->parent = mroot.get();
        mroot->args.push_back(std::unique_ptr<node>(&*child));
        return _post_order_root();
    }
}

expression_tree::post_order_iterator
expression_tree::replace_existing_node(post_order_iterator it, post_order_iterator n) noexcept {
    auto nptr = detach(n);
    auto raw = nptr.get();
    nptr->parent = it->parent;
    it->parent->args[it.get_idx()] = std::move(nptr);
    return post_order_iterator(raw);
}

expression_tree::post_order_iterator // return the next iterator
expression_tree::erase(post_order_iterator it) noexcept {
    auto original = it;
    ++it;
    if (original->parent)
        original->parent->args.erase(original->parent->args.begin() + original.get_idx());
    return it;
}

std::unique_ptr<expression_tree::node>
expression_tree::detach(post_order_iterator it) noexcept {
    if (it->parent) {
        auto num = it.get_idx(); // this is necessary
        it->parent->args[num].release();
        it->parent->args.erase(it->parent->args.begin() + num);
        return std::unique_ptr<node>(&*it);
    }
    else {
        return std::unique_ptr<node>(mroot.release());
    }
}

string expression_tree::expand(const node* ptr, string name) noexcept {
    string str = "(";
    if (ptr->args[0]->args.empty())
        str += ptr->args[0]->content.to_string() + name;
    else
        str += expand(ptr->args[0].get(), as_op(ptr->args[0]->content)->name) + name;
    for (auto it = ++ptr->args.begin(); it != ptr->args.end(); ++it) {
        if ((*it)->args.empty())
            str += (*it)->content.to_string() + name;
        else
            str += expand(it->get(), as_op((*it)->content)->name) + name;
    }
    str = str.substr(0, str.size() - name.size()) + ")";
    return str;
}

}

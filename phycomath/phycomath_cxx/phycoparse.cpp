#include "phycoparse.h"


inline string trie::node::repr() const noexcept {
    return My_repr;
}

trie::node::~node() noexcept {
    for (auto i : children)
        delete i.second;
}

inline bool trie::node::isend() const noexcept {
    return this->isEnd;
}
inline trie::node* trie::node::append(char letter) noexcept {
    if (this->findchild(letter)) {
        this->linkchild(this->findchild(letter));
        return this->findchild(letter);
    }
    else {
        auto tmp = new node(letter);
        this->linkchild(tmp);
        return tmp;
    }
}
trie::node::node() :
    letter('\0'), isroot(true), isEnd(false) {
}
trie::node::node(char letter, bool isEnd, bool isroot) :
    letter(letter), isroot(isroot), isEnd(isEnd) {
}
inline trie::node* trie::node::findchild(char letter) const noexcept {
    //for (auto i : this->children) {
    //    if (i->letter == letter) {
    //        return i;
    //    }
    //}
    auto a = children.find(letter);
    if (a != children.end())
        return a->second;
    return nullptr;
}

inline bool trie::node::haschild() const noexcept {
    return !this->children.empty();
}
inline void trie::node::linkchild(node* child) noexcept {
    this->children.insert(std::pair<char, node*>(child->letter, child));
}
inline void trie::node::markend(string str) noexcept {
    this->isEnd = true;
    My_repr = str;
}

trie::node* const trie::root = new node;

trie::trie(std::set<string> keywords) {
    node* current = root;
    for (auto i : keywords) {
        current = root;
        for (auto letter : i) {
            current = current->append(letter);
        }
        current->markend(i);
    }
    trie::reset();
}

trie::~trie() noexcept {
    delete root;
}

inline void trie::reset() noexcept {
    current = root;
    notfound.clear();
}

bool trie::put_in(char letter) {
    if (current = current->findchild(letter))
        return true;
    else {
        current = root;
        return false;
    }
}

//trie::node* trie::getnode(string str) {
//    reset();
//    for (unsigned i = 0; i < str.size(); i++) {
//        if (!put_in(str[i]))
//            return nullptr;
//    }
//    return current;
//}

std::vector<string> process(const string& input, trie& Trie) {
    Trie.reset();
    std::vector<string> ret;
    ret.reserve(100);
    //ret.reserve(input.size());
	auto it = input.begin();
    while (it != input.end()) {
        if (!Trie.notfound.empty()) {
            ret.push_back(Trie.notfound);
            Trie.reset();
        }
        if (Trie.put_in(*it)) {
            if (Trie.current->isend()) {
                ret.push_back(Trie.current->repr());
                Trie.reset();
            }
        }
        else {
            do {
                Trie.notfound += *it;
                ++it;
            } while (it != input.end() && !Trie.put_in(*it));
            continue;
        }
        ++it;
    }
    if (!Trie.notfound.empty())
        ret.push_back(Trie.notfound);
    ret.shrink_to_fit();
    return ret;
}

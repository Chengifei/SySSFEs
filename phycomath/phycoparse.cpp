#include <phycoparse.h>


string node::repr() const {
    if (parent)
        return this->parent->repr() + string(1, this->letter);
    else
        return "";
}
bool node::isend() const {
    return this->isEnd;
}
node* node::appendchar(char letter) {
    if (this->findchild(letter)) {
        this->linkchild(this->findchild(letter));
        return this->findchild(letter);
    }
    else {
        node* _ = new node(this, letter);
        this->linkchild(_);
        return _;
    }
}
node::node() :
    parent(nullptr), letter('\0'), isroot(true), isEnd(false) {
}
node::node(node* parent, char letter, bool isEnd, bool isroot) :
    parent(parent), letter(letter), isroot(isroot), isEnd(isEnd) {
}
node* node::findchild(char letter) const {
    for (auto i : this->children) {
        if (i->letter == letter) {
            return i;
        }
    }
    return nullptr;
}
bool node::haschild() const {
    return !this->children.empty();
}
void node::linkchild(node* child) {
    this->children.insert(child);
}
void node::markend() {
    this->isEnd = true;
}


namespace trie {

node* const root = new node();
node* current = root;
node* from = root;
string notfound;

void build(set<string> keywords) {
    node* current;
    for (auto i : keywords) {
        current = root;
        for (auto letter : i) {
            current = current->appendchar(letter);
        }
        current->markend();
    }
}

void reset() {
    from = trie::current;
    current = root;
    notfound.clear();
}
bool put_in(char letter) {
    from = current;
    if (current->findchild(letter)) {
        current = current->findchild(letter);
        return true;
    }
    else {
        current = root;
        return false;
    }
}
node* getnode(string str) {
    trie::reset();
    for (unsigned i = 0; i < str.size(); i++) {
        if (!trie::put_in(str.at(i)))
            return nullptr;
    }
    return trie::current;
}

}

std::vector<string> process(string input) {
    trie::reset();
    std::vector < string > ret;
    unsigned i = 0;
    while (i < input.size()) {
        if (!trie::notfound.empty()) {
            ret.push_back(trie::notfound);
            trie::reset();
        }
        if (trie::put_in(input.at(i))) {
            if (trie::current->isend()) {
                ret.push_back(trie::current->repr());
                trie::reset();
            }
        }
        else {
            do {
                trie::notfound += input.at(i);
                i++;
            } while (i < input.size() && !trie::put_in(input.at(i)));
            continue;
        }
        i++;
    }
    if (!trie::notfound.empty())
        ret.push_back(trie::notfound);
    return ret;
}

#ifdef PY_EXT
string strvec_repr(std::vector<string> vec) {
    string ret = "[";
    if (!vec.empty()) {
        for (auto i : vec) {
            ret += "'" + i + "', ";
        }
        ret.resize(ret.size() - 2);
    }
    ret += "]";
    return ret;
}
#endif
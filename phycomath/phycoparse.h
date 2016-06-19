#pragma once
#ifndef PHYCO_PARSE_H
#define PHYCO_PARSE_H
#define PY_EXT
#include <unordered_set>
#include <string>
#include <vector>
#include <stdexcept>

using std::string;
template<class T>
using set = std::unordered_set<T>;

class node {
    node* parent;
    char letter;
    bool isroot;
    bool isEnd;
    set<node*> children{};
public:
    string repr() const;
    bool isend() const;
    node* appendchar(char);
    node();
    node(node*, char, bool isEnd = false, bool isroot = false);
    node* findchild(char letter) const;
    bool haschild() const;
    void linkchild(node*);
    void markend();
};
std::vector<string> process(string);

namespace trie{
void build(set<string>);
void reset();
node* getnode(string);
}
#ifdef PY_EXT
string strvec_repr(std::vector<string>);
#endif
#endif
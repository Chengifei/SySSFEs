#pragma once
#ifndef PHYCO_PARSE_H
#define PHYCO_PARSE_H
#include <map>
#include <set>
#include <string>
#include <vector>

using std::string;

class trie {
    class node {
        std::map<char, node*> children;
        //std::vector<node*> children{};
        string My_repr;
        char letter;
        bool isroot;
        bool isEnd;
    public:
        bool isend() const noexcept;
        node* append(char) noexcept;
        node();
        explicit node(char, bool isEnd = false, bool isroot = false);
        node* findchild(char) const noexcept;
        bool haschild() const noexcept;
        void linkchild(node*) noexcept;
        void markend(string) noexcept;
        string repr() const noexcept;
        ~node() noexcept;
    };
public:
    trie(std::set<string>);
    ~trie() noexcept;
    static node* const root;
    node* current;
    string notfound;
    bool put_in(char);
    void reset() noexcept;
};

std::vector<string> process(const string&, trie&);
#endif

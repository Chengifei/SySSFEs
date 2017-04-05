#ifndef SET_SUPPORT_HPP
#define SET_SUPPORT_HPP
#include <unordered_set>
#include <boost/container/flat_set.hpp>
#include <boost/functional/hash.hpp>

template <typename T, class hash = boost::hash<T>>
using CSF_set = std::unordered_set<T, hash>;

template <typename T, class... U>
using CSF_flat_set = boost::container::flat_set<T, U...>;

template <class C1, class C2>
bool is_subset(const C1& S1, const C2& S2) {
    for (const auto& i : S1) {
        if (!S2.count(i))
            return false;
    }
    return true;
}

template <class C1, class C2>
void update(C1& S1, const C2& S2) {
    for (auto& i: S2)
        S1.insert(i);
}

template <class C>
C union_sets(const C& S1, const C& S2) {
    C ret = S1;
    for (auto& i: S2)
        ret.insert(i);
    return ret;
}

template <class C>
C intersect_sets(const C& S1, const C& S2) {
    C ret;
    for (const auto& i: S1)
        if (S2.count(i))
            ret.insert(i);
    return ret;
}

template <class C1, class C2>
void exclude(C1& S1, const C2& S2) noexcept {
    for (const auto& i: S2)
        S1.erase(i);
}

template <class C, class T>
bool verify_then_remove(C& set, const T& key) {
    auto it = set.find(key);
    if (it != set.end()) {
        set.erase(it);
        return true;
    }
    return false;
}
#endif

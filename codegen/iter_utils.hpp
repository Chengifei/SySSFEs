#ifndef ITER_UTILS_HPP
#define ITER_UTILS_HPP
#include <memory>
#include <vector>
#include <utility>
#include <type_traits>

namespace iter_utils {

struct None {};

template <class BI>
struct hack_iterator {
    BI& base_iterator;
    explicit hack_iterator(BI& bi) : base_iterator(bi) {}
    bool operator!=(None) const {
        return !base_iterator.exhausted();
    }
    void operator++() {
        ++base_iterator;
    }
    decltype(auto) operator*() {
        return *base_iterator;
    }
};

template <class T>
class non_trivial_end_iter {
public:
    hack_iterator<T> begin() {
        return hack_iterator<T>(*static_cast<T*>(this));
    }
    None end() {
        return None();
    }
};

template <class T>
struct as_array {
    typedef typename std::remove_pointer<T>::type ref_type;
    T _begin;
    T _end;
    as_array(std::pair<T, T> p) : _begin(p.first), _end(p.second) {};
    ref_type operator[](int idx) const {
        return *(_begin + idx);
    }
    std::size_t size() const {
        return _end - _begin;
    }
    T begin() const {
        return _begin;
    }
    T end() const {
        return _end;
    }
};

}

template <typename T>
class range : public iter_utils::non_trivial_end_iter<range<T>> {
    T current;
    T terminal;
public:
    range(T begin, T end) : current(begin), terminal(end) {}
    void operator++() {
        ++current;
    }
    T operator*() {
        return current;
    }
    bool exhausted() {
        return current == terminal;
    }
};

template <class T>
struct combination : iter_utils::non_trivial_end_iter<combination<T>> {
    typedef typename std::iterator_traits<T>::value_type value_type;
    const std::size_t num_in_pool;
    std::unique_ptr<value_type[]> pool;
    std::unique_ptr<std::size_t[]> indices;
    /* const */ std::size_t* indices_end;
    std::unique_ptr<value_type[]> result;
    std::size_t* cursor;
    combination(T begin, T end, std::size_t sz)
            : num_in_pool(std::distance(begin, end)) {
        result = std::make_unique<value_type[]>(sz);
        indices = std::make_unique<std::size_t[]>(sz);
        for (auto i : range<std::size_t>(0, sz)) {
            indices[i] = i;
            result[i] = std::move(*(begin++));
        }
        indices_end = indices.get() + sz;
        cursor = indices_end - 1;
        pool = std::make_unique<value_type[]>(num_in_pool);
        for (auto i : range<std::size_t>(sz, num_in_pool))
            pool[i] = std::move(*(begin++));
    }
    combination& operator++() {
        while (*cursor + (indices_end - cursor) == num_in_pool) {
            --cursor;
            if (cursor - indices.get() == -1) {
                result = nullptr;
                return *this;
            }
        }
        for (std::size_t* ptr = cursor; ptr != indices_end; ++ptr)
            pool[*ptr] = std::move(result[ptr - indices.get()]);
        ++*cursor;
        result[cursor - indices.get()] = std::move(pool[*cursor]);
        for (++cursor; cursor != indices_end; ++cursor) {
            *cursor = *(cursor - 1) + 1;
            result[cursor - indices.get()] = std::move(pool[*cursor]);
        }
        --cursor;
        return *this;
    }
    bool exhausted() {
        return !static_cast<bool>(result);
    }
    const value_type* operator*() {
        return result.get();
    }
};

template <class T>
struct powerset : iter_utils::non_trivial_end_iter<powerset<T>> {
    typedef typename T::value_type value_type;
    const T& pool; // TODO: Make a copy here, and all further are moves.
    std::vector<value_type> result;
    int idx = 0;
    const std::size_t sz;
    std::unique_ptr<std::size_t[]> indices;
    powerset(const T& pool, std::size_t sz)
        : pool(pool), sz(sz), indices(std::make_unique<std::size_t[]>(sz)) {
        indices[0] = 0;
        result.reserve(sz);
        resize();
    }
    powerset& operator++() {
        if (indices[idx] + (result.size() - idx) != pool.size()) {
            ++indices[idx];
            result[idx] = pool[indices[idx]];
            return *this;
        }
        while (indices[idx] + (result.size() - idx) == pool.size()) {
            --idx;
            if (idx == -1) {
                resize();
                return *this;
            }
        }
        ++indices[idx];
        result[idx] = pool[indices[idx]];
        for (unsigned _idx = idx + 1; _idx != result.size(); ++_idx) {
            indices[_idx] = indices[_idx - 1] + 1;
            result[_idx] = pool[indices[_idx]];
        }
        idx = result.size() - 1;
        return *this;
    }
    bool exhausted() {
        return result.empty();
    }
    std::vector<value_type>& operator*() {
        return result;
    }
    std::vector<value_type>* operator->() {
        return &result;
    }
    const std::size_t* raw() const noexcept {
        return indices.get();
    }
private:
    void resize() {
        if (result.size() != sz) {
            idx = result.size();
            result.resize(idx + 1);
            for (unsigned _idx = 0; _idx != result.size(); ++_idx) {
                indices[_idx] = _idx;
                result[_idx] = pool[_idx];
            }
        }
        else {
            result.clear();
        }
    }
};

template <class TIT, class UIT>
struct product : iter_utils::non_trivial_end_iter<product<TIT, UIT>> {
    typedef typename TIT::value_type T;
    typedef typename UIT::value_type U;
    TIT begin1, it1, end1;
    UIT begin2, it2, end2;
    product(TIT begin1, TIT end1, UIT begin2, UIT end2) 
        : begin1(begin1), it1(begin1), end1(end1), begin2(begin2), it2(begin2), end2(end2) {}
    product& operator++() {
        if (it1 != end1)
             ++it1;
        if (it1 == end1) {
            ++it2;
            it1 = begin1;
        }
        return *this;
    }
    std::pair<T, U> operator*() {
        return std::make_pair(*it1, *it2);
    }
    bool exhausted() const {
        return it2 == end2;
    }
};
#endif

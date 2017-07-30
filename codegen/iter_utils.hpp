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
struct random_iterator_view {
    typedef typename T::reference ref_type;
    T begin;
    T end;
    ref_type operator[](int idx) const {
        return *(begin + idx);
    }
    std::size_t size() const {
        return end - begin;
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
    typedef typename T::value_type value_type;
    const iter_utils::random_iterator_view<T> pool;
    std::vector<std::size_t> indices;
    std::vector<value_type> result;
    const std::size_t sz;
    combination(T begin, T end, std::size_t sz)
        : pool{begin, end}, sz(sz) {
        indices.reserve(sz);
        result.reserve(sz);
        for (auto i : range<std::size_t>(0, sz)) {
            indices.push_back(i);
            result.push_back(pool[i]);
        }
    }
    combination& operator++() {
        std::size_t idx = sz - 1;
        while (indices[idx] + (sz - idx) == pool.size()) {
            if (idx == 0) {
                result.clear();
                return *this;
            }
            --idx;
        }
        ++indices[idx];
        result[idx] = pool[indices[idx]];
        for (std::size_t _idx = idx + 1; _idx != sz; ++_idx) {
            indices[_idx] = indices[_idx - 1] + 1;
            result[_idx] = pool[indices[_idx]];
        }
        return *this;
    }
  	bool exhausted() {
      	return result.empty();
    }
  	std::vector<value_type>& operator*() {
      	return result;
    }
};

template <class T>
struct powerset : iter_utils::non_trivial_end_iter<powerset<T>> {
    typedef typename T::value_type value_type;
    const T& pool;
    std::vector<value_type> result;
    int idx = 0;
    const std::size_t sz;
    std::unique_ptr<std::size_t[]> indices;
    powerset(const T& pool, std::size_t sz)
        : pool(pool), sz(sz), indices(std::make_unique<std::size_t[]>(sz)) {
        indices[0] = 0;
        result.reserve(sz);
        result.resize(1);
        result[idx] = pool[indices[idx]];
    }
    powerset& operator++() {
        if (indices[idx] + (result.size() - idx) != sz) {
            ++indices[idx];
            result[idx] = pool[indices[idx]];
            return *this;
        }
        while (indices[idx] + (result.size() - idx) == sz) {
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

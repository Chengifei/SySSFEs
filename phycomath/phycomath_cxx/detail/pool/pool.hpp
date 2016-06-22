#ifndef PHYCO_DETAIL_POOL_HPP
#define PHYCO_DETAIL_POOL_HPP
#include <cassert>
#include <cstdlib>
#include <algorithm>

namespace utils {

template <class T, unsigned max>
class pool {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    pool() {}
    template <class U>
    pool(const pool<U, max>& other) {}
    template <class U>
    struct rebind { typedef pool<U, max> other; };
private:
    static T* min;
    static T* current;
    static inline void get_buffer() {
        min = (T*) ::operator new(sizeof(T)*max);
        current=min;
    }
public:
    static inline T* allocate(std::size_t n = 1) {
        if (!min)
            get_buffer();
        if (current-min<max) {
            return current++;
        }
        else {
            return (T*) ::operator new(sizeof(T)*n);
        }
    }
    static inline void deallocate(T* ptr, std::size_t n = 1) noexcept {
        if (ptr){
            for (auto tmp=ptr; n != 0; --n, ++tmp)
                tmp->~T();
            if (min<=ptr && ptr+(n-1)-min<=max){
                if (current-ptr==1)
                    current=ptr;
            }
            else
                ::operator delete(ptr, sizeof(T)*n);
        }
    }
    static inline void purge() noexcept {
        ::operator delete(min, sizeof(T)*max);
        min=nullptr;
    }
    constexpr static unsigned max_size() noexcept {
        return max;
    }
};

template <class T, unsigned max>
T* pool<T,max>::min=nullptr;
template <class T, unsigned max>
T* pool<T,max>::current=nullptr;

}
#endif

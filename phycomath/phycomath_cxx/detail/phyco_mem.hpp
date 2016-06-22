/* Copyright (c) 2016 by Yifei Zheng
 * All rights Reserved.
 */
#ifndef PHYCO_MEM_H
#define PHYCO_MEM_H
#define PHYCO_DEFAULT_POOL_SIZE 0x10000U
#ifdef PHYCO_DEBUG
#include <cassert>
#endif
#include <vector>
#include "pool/pool.hpp"
#include "quad_list/quad_list.hpp"

#define EXPORT_THIS

namespace utils {

class memory_manager {
protected:
    template <class T>
    static inline void free(T* ptr, std::size_t n = 1) {
        data.push(intervalp((uintptr_t)ptr, (uintptr_t)ptr + n*sizeof(T)));
    }
    template <class T>
    static inline T* alloc(std::size_t n, unsigned align) {
        return reinterpret_cast<T*>(search_size(n * sizeof(T), align));
    }
    static inline void free_all() noexcept {
        data.clear();
        for (auto i : pool)
            delete[](char*)i;
    }
private:
    static EXPORT_THIS quad_list data;
    static std::vector<void*> pool;
    //std::size_t default_new_size;
    static EXPORT_THIS void* search_size(std::size_t, unsigned);
    static EXPORT_THIS void get_new(std::size_t size = PHYCO_DEFAULT_POOL_SIZE);
};

template <typename T, std::size_t pool_size = PHYCO_DEFAULT_POOL_SIZE>
class unified_allocator : public memory_manager {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
public:
    unified_allocator() {}
    template <class U>
    unified_allocator(const unified_allocator<U>& other) {}
    template <class U>
    struct rebind { typedef unified_allocator<U> other; };
public:
    static inline pointer allocate(std::size_t n) {
        return memory_manager::alloc<T>(n, alignof(T));
    }
    static inline void deallocate(T* p, std::size_t n) {
        if (p) {
            p->~T();
            memory_manager::free<T>(p, n);
        }
    }
public:
    static inline pointer malloc(std::size_t n) {
        return memory_manager::alloc<T>(n, alignof(T));
    }
};

template <class T>
//using allocator = unified_allocator<T>;
using allocator = pool<T, 1000>;
//using allocator=std::allocator<T>;

//template class EXPORT_THIS allocator<int>;

}
#endif

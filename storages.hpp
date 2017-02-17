#ifndef STACK_HPP
#define STACK_HPP
#include "env.hpp"
#include <cstdint>
#include <cstddef>
#include <new>
#include <type_traits>

template <std::size_t A, std::size_t S>
class stack {
    typename std::aligned_storage<S, A>::type content;
    std::uintptr_t top = reinterpret_cast<std::uintptr_t>(&content);
public:
    template <typename T>
    void push(T t) noexcept {
        *reinterpret_cast<T*>(top) = std::move(t);
        top += aligned<T>();
    }
    void push(const char* t, std::size_t alignment, std::size_t sz) {
        std::memcpy(reinterpret_cast<void*>(top), t, sz);
        top += aligned(sz, alignment);
    }
  	template <typename T>
    T&& pop() noexcept {
        top -= aligned<T>();
        return std::move(*reinterpret_cast<T*>(top));
    }
    char* pop(std::size_t alignment, std::size_t sz) {
        top -= aligned(sz, alignment);
        return reinterpret_cast<char*>(top);
    }
    template <typename T>
    T& peek() noexcept {
        return *reinterpret_cast<T*>(top - aligned<T>());
    }
    template <typename T>
    T& peek(unsigned i) noexcept {
        return *(reinterpret_cast<T*>(&content) + i);
    }
    char* peek(unsigned i) noexcept {
        return reinterpret_cast<char*>(&content) + A * i;
    }
private:
    template <typename T>
    constexpr static std::size_t aligned() {
        return sizeof(T) > alignof(T) ? sizeof(T) : alignof(T);
    }
    constexpr static std::size_t aligned(std::size_t sz, std::size_t align) {
        unsigned mask = align - 1;
  	    if (sz & mask)
      		return (sz | mask) + 1;
        return sz;
    }
};

template <std::size_t A, std::size_t S>
class dstack {
    void* base;
    char* top;
public:
    dstack() {
        base = ::operator new(S, static_cast<std::align_val_t>(A));
        top = reinterpret_cast<char*>(base);
    }
    void write(const char* t, std::size_t alignment, std::size_t sz) {
        std::memcpy(top, t, sz);
        top += aligned(sz, alignment);
    }
    void clear() {
        top = reinterpret_cast<char*>(base);
    }
    char* operator[](std::size_t idx) {
        return reinterpret_cast<char*>(base) + A * idx;
    }
private:
    template <typename T>
    constexpr static std::size_t aligned() {
        return sizeof(T) > alignof(T) ? sizeof(T) : alignof(T);
    }
    constexpr static std::size_t aligned(std::size_t sz, std::size_t align) {
        unsigned mask = align - 1;
  	    if (sz & mask)
      		return (sz | mask) + 1;
        return sz;
    }
};
#endif

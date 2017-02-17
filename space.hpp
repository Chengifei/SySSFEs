#ifndef SPACE_HPP
#define SPACE_HPP
#include <stdexcept>
#include <cstring>
#include "storages.hpp"

template <typename T>
struct false_helper : std::false_type {};

struct shared {
private:
    stack<32, 512> stack32;
    stack<8, 512> stack8;
    dstack<32, 1024> mem32;
    dstack<8, 1024> mem8;
    char flags[8]; // Never exceed 8 flags, no complex conditions
    unsigned fidx = 0;
    const unsigned total_size;
    const std::size_t size;
    void* storage;
public:
    shared(std::size_t align, unsigned n, std::size_t size) :
        total_size(n * size), size(size) {
        storage = ::operator new(total_size, static_cast<std::align_val_t>(align));
    }
    const char* init(const char* it) {
        std::memcpy(reinterpret_cast<void*>(storage), it, total_size);
        return it + size;
    }
    ~shared() {
        // This is not actually necessary, since this will only be called on exit.
        ::operator delete(storage, size);
    }
#define REJECT static_assert(false_helper<T>::value, "There's no corresponding stack.")
    template <typename T>
    void push(T t) noexcept {
        if constexpr (alignof(T) == 8)
            stack8.template push<T>(t);
        else if constexpr (alignof(T) == 32)
            stack32.template push<T>(t);
        else
            REJECT;
    }
    void push(const char* t, std::size_t alignment, std::size_t sz) {
        if (alignment == 8)
            stack8.push(t, alignment, sz);
        else if (alignment == 32)
            stack32.push(t, alignment, sz);
        else
            throw std::runtime_error("Runtime dynamic stack access with unknown alignment");
    }
  	template <typename T>
    T&& pop() noexcept {
        if constexpr (alignof(T) == 8)
            return stack8.template pop<T>();
        else if constexpr (alignof(T) == 32)
            return stack32.template pop<T>();
        else
            REJECT;
    }
    char* pop(std::size_t alignment, std::size_t sz) {
        if (alignment == 8)
            return stack8.pop(alignment, sz);
        else if (alignment == 32)
            return stack32.pop(alignment, sz);
        else
            throw std::runtime_error("Runtime dynamic stack access with unknown alignment");
    }
    template <typename T>
    T& peek() noexcept {
        if constexpr (alignof(T) == 8)
            return stack8.template peek<T>();
        else if constexpr (alignof(T) == 32)
            return stack32.template peek<T>();
        else
            REJECT;
    }
    template <typename T>
    T& peek(unsigned i) noexcept {
        if constexpr (alignof(T) == 8)
            return stack8.template peek<T>(i);
        else if constexpr (alignof(T) == 32)
            return stack32.template peek<T>(i);
        else
            REJECT;
    }
    char* peek(unsigned i, std::size_t alignment) noexcept {
        if (alignment == 8)
            return stack8.peek(i);
        else if (alignment == 32)
            return stack32.peek(i);
        else
            throw std::runtime_error("Runtime dynamic stack access with unknown alignment");
    }
    char popf() {
        return flags[--fidx];
    }
    char peekf() const {
        return flags[fidx - 1];
    }
    void pushf(char t) {
        flags[fidx++] = t;
    }
    char* get_obj(unsigned idx) {
        return reinterpret_cast<char*>(storage) + idx * size;
    }
    void store(const char* t, std::size_t alignment, std::size_t sz) {
        if (alignment == 8)
            mem8.write(t, alignment, sz);
        else if (alignment == 32)
            mem32.write(t, alignment, sz);
        else
            throw std::runtime_error("Runtime dynamic free store access with unknown alignment");
    }
    char* read(std::size_t alignment, unsigned idx) {
        if (alignment == 8)
            return mem8[idx];
        else if (alignment == 32)
            return mem32[idx];
        else
            throw std::runtime_error("Runtime dynamic free store access with unknown alignment");
    }
    void clear () {
        mem8.clear();
        mem32.clear();
    }
};
#endif

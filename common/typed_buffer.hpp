#ifndef TYPED_BUFFER_HPP
#define TYPED_BUFFER_HPP
#include <cstddef>
#include <type_traits>
#include <memory>

template <typename T, std::size_t sz>
class typed_buffer {
    typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type raw_type;
    std::unique_ptr<raw_type[]> buffer;
public:
    typed_buffer() {
        buffer = std::make_unique<raw_type[]>(sz);
    }
    T& operator[](std::size_t idx) {
        return *reinterpret_cast<T*>(&buffer[idx]);
    }
    T* begin() noexcept {
        return &operator[](0);
    }
    T* end() noexcept {
        return &operator[](sz);
    }
};

template <typename T>
class placeholder {
    alignas(alignof(T)) unsigned char buffer[sizeof(T)];
public:
    T* operator->() noexcept {
        return reinterpret_cast<T*>(buffer);
    }
    T& operator*() noexcept {
        return *reinterpret_cast<T*>(buffer);
    }
    T* get_addr() noexcept {
        return reinterpret_cast<T*>(buffer);
    }
};
#endif

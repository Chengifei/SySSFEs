#ifndef READER_HPP
#define READER_HPP
#include <cstddef>

namespace detail {
    template <typename T, typename P>
    const T& read_as(const P*& ptr) {
        auto tmp = ptr;
        ptr += sizeof(T);
        return *reinterpret_cast<const T*>(tmp);
    }
    template <typename T, typename P>
    T read_sized(const P*& ptr, std::size_t sz) {
        auto tmp = ptr;
        ptr += sz;
        return *reinterpret_cast<const T*>(tmp);
    }
}
#endif

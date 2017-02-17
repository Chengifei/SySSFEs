#ifndef MAP_HPP
#define MAP_HPP
#include <cstddef>

struct Map {
    typedef const char* const_iterator;
    typedef const_iterator iterator;
    const char* start;
    std::size_t size;
    explicit Map(const char* const start, std::size_t size = 0) :
        start(start), size(size) {}
    const char* begin() const {
        return start;
    }
    const char* end() const {
        return start + size;
    }
    bool empty() const {
        return size;
    }
};
#endif

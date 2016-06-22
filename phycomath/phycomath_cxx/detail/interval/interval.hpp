#ifndef PHYCO_DETAIL_INTERVAL_HPP
#define PHYCO_DETAIL_INTERVAL_HPP
#include <cstdint>

namespace utils {

template <typename int_t = int>
struct interval {
    // this is not the interval on mathematics
    int_t low;
    int_t size;
    interval(int_t a, int_t b) : low(a), size(b - a) {};
    bool operator==(interval other) const noexcept {
        return other.low == low && other.size == size;
    }
    bool operator<(interval other) const noexcept {
        return low < other.low;
    }
    bool operator<(int_t other) const noexcept {
        return low < other;
    }
    inline bool contains(int_t other) const noexcept {
        return low <= other && other < low + size;
    }
    inline void Lmerge(interval other) noexcept {
        low = other.low;
        size += other.size;
    }
    inline void Rmerge(interval other) noexcept {
        size += other.size;
    }
};

typedef interval<uintptr_t> intervalp;

}
#endif

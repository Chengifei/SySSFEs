/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This header defines a general class that produces fixed-length combinations.
 */
#ifndef COMBINATION_HPP
#define COMBINATION_HPP
#include <cstddef>

template <std::size_t size, std::size_t start, std::size_t stop>
class combinations {
    std::size_t guard = -1ULL;
    std::size_t current[size];
public:
    combinations() noexcept {
        reset();
    }
    void operator++() {
        std::size_t last = size - 1;
        // clang was not able to separate constant to the other side
        while (current[last] - last == stop - size) // current[last] + (size - last) == stop
            --last;
        for (std::size_t first = current[last] + 1; last != size;
             ++last, ++first)
            current[last] = first;
    }
    std::size_t operator[](std::size_t i) const {
        return current[i];
    }
    bool exhausted() const {
        return !current[-1];
    }
    void reset() {
        for (std::size_t i = start, idx = 0; idx != size; ++i, ++idx)
            current[idx] = i;
    }
};
#endif

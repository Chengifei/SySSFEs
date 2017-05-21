/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This header defines a general class that produce fixed-length combination.
 */
#ifndef COMBINATION_HPP
#define COMBINATION_HPP
#include <cstddef>

template <std::size_t size>
struct combinations {
    const std::size_t start, stop;
    std::size_t current[size];
    combinations(std::size_t start, std::size_t stop) noexcept
        : start(start), stop(stop) {
        reset();
    }
    // end condition: current[0] + size == stop
    void operator++() noexcept {
        for (std::size_t i = size - 1; i >= 0; --i) {
            if (current[i] + (size - i) != stop) {
                ++current[i];
                reinit(i);
                return;
            }
        }
    }
    std::size_t get(std::size_t i) const noexcept {
        return current[i];
    }
    void reset() noexcept {
        std::size_t it = start;
        for (std::size_t i = 0; i != size; ++i)
            current[i] = it++;
    }
private:
    void reinit(std::size_t i) noexcept {
        std::size_t first = current[i++];
        for (; i != size; ++i)
            current[i] = ++first;
    }
};
#endif
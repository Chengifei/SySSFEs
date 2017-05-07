/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This header defines a general class that produce fixed-length combination.
 */
#ifndef COMBINATION_HPP
#define COMBINATION_HPP
template <unsigned size, class T = unsigned>
struct combinations {
    T start;
    T stop;
    T current[size];
    combinations(T start, T stop):
        start(start), stop(stop) {
        reset();
    }
    void operator++() {
        return next(size - 1);
    }
    const T& get(unsigned i) const {
        return current[i];
    }
    void reset() {
        auto it = start;
        for (unsigned i = 0; i != size; ++i)
            current[i] = it++;
    }
private:
  	void next(unsigned i) {
        if (current[0] + size == stop)
            current[0] = stop;
        else {
          	for (; i >= 0; --i) {
              	if (current[i] + (size - i) != stop) {
              		++current[i];
          			break;
                }
            }
  	        reinit(i);
        }
    }
  	void reinit(unsigned i) {
      	T first = current[i++];
        for (; i != size; ++i)
          	current[i] = ++first;
    }
};
#endif

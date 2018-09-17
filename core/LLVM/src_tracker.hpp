#ifndef SRC_TRACKER_HPP
#define SRC_TRACKER_HPP
#include <support/src_location.hpp>
template <typename T>
struct with_location : support::src_location, T {};
#endif

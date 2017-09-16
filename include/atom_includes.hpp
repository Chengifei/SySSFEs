/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM.
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This is the meta-header for any ATOM project.
 */

#ifndef ATOM_INCLUDES_HPP
#define ATOM_INCLUDES_HPP
#include "env.hpp"
#include "combination.hpp"
#include <memory>
#include <vector>
#include <fstream>
#include "math/solvers.hpp"
#include "math/calculus.hpp"
#include "math/operators.hpp"

namespace types {

template <typename T>
using log = std::vector<T>;

}
#endif

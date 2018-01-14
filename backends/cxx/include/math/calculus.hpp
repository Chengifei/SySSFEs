/* Copyright 2017-2018 by Yifei Zheng
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This header defines differentiation functions and reciprocal 
 * differentiation routines.
 */

#ifndef CALCULUS_HPP
#define CALCULUS_HPP
#include <utility>
#include <cmath>

namespace math { namespace calculus {

template <class F>
double fdiff(const F& expr, double x) {
    double step = (fabs(x) > 1) ? 0x1p-20 * x :
                                  0x1p-20;
    auto y1 = expr(x - step);
    auto y2 = expr(x + step);
    return (y2 - y1) / (step + step);
}

template <class F, class... Gs>
double fdiff(const F& expr, double step, Gs&&... getters) {
    auto y1 = expr(std::forward<Gs&&>(getters)(0)...);
    auto y2 = expr(std::forward<Gs&&>(getters)(1)...);
    return (y2 - y1) / step;
}


template <class F, class... Args>
double rdiff(const F& expr, double x, Args... args) {
    double step = (fabs(x) > 1) ? 0x1p-20 * x :
                                  0x1p-20;
    auto y1 = expr(x - step, args...);
    auto y2 = expr(x + step, args...);
    return (step + step) / (y2 - y1);
}

}}
#endif

#ifndef CALCULUS_HPP
#define CALCULUS_HPP
#include <utility>

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

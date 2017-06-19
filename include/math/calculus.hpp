#ifndef CALCULUS_HPP
#define CALCULUS_HPP

namespace math { namespace calculus {

template <class F, class... Args>
double fdiff(const F& expr, double x, Args... args) {
    double step = (fabs(x) > 1) ? 0x1p-20 * x :
                                  0x1p-20;
    auto y1 = expr(x - step, args...);
    auto y2 = expr(x + step, args...);
    return (y2 - y1) / (step + step);
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

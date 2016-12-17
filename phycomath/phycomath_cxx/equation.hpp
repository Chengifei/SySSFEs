#ifndef EQUATION_HPP
#define EQUATION_HPP
#include "expression_tree.hpp"

namespace math {
class eqn_t {
private:
    expression_tree lhs, rhs;
public:
    eqn_t(expression_tree&&, expression_tree&&);
    //eqn_t(const eqn_t&) = delete;
    void group_to_left();
#ifdef PY_EXT
    double fNsolve(string varname, PyObject* dict);
#endif
    bool verify(const map<string, double>&);
    inline string repr() const noexcept {
        return lhs.repr() + "=" + rhs.repr();
    }
};
}
#endif

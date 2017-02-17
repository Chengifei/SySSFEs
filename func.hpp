#ifndef FUNC_HPP
#define FUNC_HPP
#include "Map.hpp"
#include "space.hpp"

class func {
    Map cond;
  	Map instrs;
    friend struct rule_iterator;
    func() : cond(nullptr), instrs(nullptr) {}
public:
    explicit func(Map instrs) : cond(nullptr), instrs(instrs) {}
    func(Map cond, Map instrs) : cond(cond), instrs(instrs) {}
    void operator()(shared& srd) const {
        exec(srd, instrs.begin(), instrs.end());
    }
    template <typename T>
    T eval(shared& srd) const {
        exec(srd, instrs.begin(), instrs.end());
        return srd.pop<T>();
    }
    bool enable(shared& srd) const {
        if (!cond.start)
            return true;
        else {
            exec(srd, cond.begin(), cond.end());
            return srd.popf();
        }
    }
private:
    static void exec(shared&, Map::const_iterator begin, Map::const_iterator end);
};

#endif

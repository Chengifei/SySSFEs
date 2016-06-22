#ifndef PHYCO_DETAIL_VARIANT_HPP
#define PHYCO_DETAIL_VARIANT_HPP
#include <string>
#ifdef PHYCO_DEBUG
#include <cassert>
#endif

namespace math {
struct Op;
}

namespace utils {
namespace variant {
struct argument {
    union {
        int i = 0;
        double d;
        std::string s;
        const math::Op* o;
    };
    enum { INT, DOUBLE, STR, OP } tag = INT;
    argument() {}
    argument(int i) :i(i), tag(INT) {}
    argument(double d) :d(d), tag(DOUBLE) {}
    explicit argument(std::string s) :s(s), tag(STR) {}
    explicit argument(const math::Op* o) : o(o), tag(OP) {}
    argument(const argument&) = delete;
    argument(argument&& other) noexcept {
        switch (other.tag) {
            case 0:
                this->operator=(other.i);
                break;
            case 1:
                this->operator=(other.d);
                break;
            case 2:
                this->operator=(std::move(other.s));
                break;
            case 3:
                this->operator=(other.o);
                break;
        }
    }
    bool is_expr() const noexcept {
        return tag == 3;
    }
    argument& operator=(int i) noexcept {
        this->i = i;
        tag = INT;
        return *this;
    }
    argument& operator=(double d) noexcept {
        this->d = d;
        tag = DOUBLE;
        return *this;
    }
    argument& operator=(std::string s) noexcept {
        if(tag!=STR){
            new(&this->s) std::string(s);
            tag = STR;
        }
        else
            this->s=s;
        return *this;
    }
    argument& operator=(const math::Op* o) noexcept {
        this->o = o;
        tag = OP;
        return *this;
    }
public:
    argument& operator=(argument&& other) noexcept {
        this->~argument();
        switch (other.tag) {
            case 0:
                return this->operator=(other.i);
            case 1:
                return this->operator=(other.d);
            case 2:
                return this->operator=(std::move(other.s));
            case 3:
                return this->operator=(other.o);
        }
    }
    //argument& operator=(const argument&) = delete;
    ~argument() noexcept {
        if (tag == 2)
            s.~basic_string();
    }
private:
    void* get_whatever() noexcept {
        return &this->i;
    }
};

template <class V_T>
inline typename V_T::result_type apply_visitor(const V_T& visitor, argument& arg) {
    switch (arg.tag) {
        case 0:
            return visitor(arg.i);
        case 1:
            return visitor(arg.d);
        case 2:
            return visitor(arg.s);
        case 3:
            return visitor(arg.o);
    }
    throw;
}

template <class V_T>
inline typename V_T::result_type apply_visitor(const V_T& visitor, const argument& arg) {
    switch (arg.tag) {
        case 0:
            return visitor.operator()(arg.i);
        case 1:
            return visitor.operator()(arg.d);
        case 2:
            return visitor.operator()(arg.s);
        case 3:
            return visitor.operator()(arg.o);
    }
    throw;
}

template<class T = void>
struct static_visitor {
    using result_type = T;
};

}
}
#endif

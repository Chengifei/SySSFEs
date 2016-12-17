#include "detail/static_dispatcher.hpp"
#include "operators.hpp"
#ifdef PY_EXT
#include "boost/python/dict.hpp"
#include "boost/python/extract.hpp"
#endif
#include <string>
using std::string;

namespace math {

class arg_base {
public:
    virtual arg_base* clone() const = 0;
    virtual double to_double(const map<string, double>&) const = 0;
#ifdef PY_EXT
    virtual double to_double(const boost::python::dict&) const = 0;
#endif
    virtual string to_string() const = 0;
    virtual ~arg_base() noexcept {}
};

class op_wrapper : public arg_base {
    const Op* op;
public:
    op_wrapper(const Op* op) : op(op) {}
    op_wrapper* clone() const override {
        return new op_wrapper{ op };
    }
#ifdef PY_EXT
    [[noreturn]] double to_double(const boost::python::dict&) const override {
        abort();
    }
#endif
    [[noreturn]] double to_double(const map<string, double>&) const override {
        abort();
    }
    string to_string() const override {
        return op->name;
    }
    const Op* const get_ptr() const noexcept {
        return op;
    }
    bool operator==(const op_wrapper& rhs) const {
        return op == rhs.op;
    }
    ~op_wrapper() noexcept override {}
};

class int_wrapper : public arg_base {
public:
    int i;
    int_wrapper(int i) : i(i) {}
    int_wrapper* clone() const override {
        return new int_wrapper(i);
    }
    double to_double(const map<string, double>&) const override {
        return i;
    }
#ifdef PY_EXT
    double to_double(const boost::python::dict&) const override {
        return i;
    }
#endif
    string to_string() const override {
        return std::to_string(i);
    }
    bool operator==(const int_wrapper& rhs) const {
        return i == rhs.i;
    }
    ~int_wrapper() noexcept override {}
};

class double_wrapper : public arg_base {
public:
    double d;
    double_wrapper(double d) : d(d) {}
    double_wrapper* clone() const override {
        return new double_wrapper(d);
    }
    double to_double(const map<string, double>&) const override {
        return d;
    }
#ifdef PY_EXT
    double to_double(const boost::python::dict&) const override {
        return d;
    }
#endif
    string to_string() const override {
        return std::to_string(d);
    }
    bool operator==(const double_wrapper& rhs) const {
        return d == rhs.d;
    }
    ~double_wrapper() noexcept override {}
};

class string_wrapper : public arg_base {
    string s;
public:
    string_wrapper(string s) : s(std::move(s)) {}
    string_wrapper* clone() const override {
        return new string_wrapper(s);
    }
#ifdef PY_EXT
    double to_double(const boost::python::dict& dict) const override {
        return boost::python::extract<double>(dict.get(s));
    }
#endif
    double to_double(const map<string, double>& dict) const override {
        return dict.at(s);
    }
    string to_string() const override {
        return s;
    }
    bool operator==(const string_wrapper& rhs) const {
        return s == rhs.s;
    }
    ~string_wrapper() noexcept override {}
};

template<typename T>
concept bool EqualityComparable = requires(const T& a, const T& b) {
    { a == b } -> bool;
};

struct compare_visitor {
    static constexpr bool symmetric = true;
    template <class T, class U>
    constexpr bool run(const T&, const U&) {
        return false;
    }
    template <class T>
    constexpr bool run(const T& lhs, const T& rhs) requires EqualityComparable<T> {
        return lhs == rhs;
    }
};

template <>
constexpr bool compare_visitor::run<int, double>(const int& lhs, const double& rhs) {
    return lhs == rhs;
}

class arg_t {
    using compare_dispatcher =
        utils::static_dispatcher
        <
        compare_visitor,
        arg_base,
        utils::TL::typelist<int_wrapper, double_wrapper, string_wrapper, op_wrapper>,
        bool
        >;
    arg_base* ptr;
public:
    arg_t(arg_base* ptr) : ptr(ptr) {}
    explicit arg_t(const Op* op) : ptr(new op_wrapper(op)) {}
    explicit arg_t(int i) : ptr(new int_wrapper(i)) {}
    explicit arg_t(double d) : ptr(new double_wrapper(d)) {}
    explicit arg_t(string s) : ptr(new string_wrapper(std::move(s))) {}
    arg_t(const arg_t& rhs) : ptr(rhs.ptr->clone()) {}
    arg_t& operator=(const arg_t& rhs) {
        ptr = rhs.ptr->clone();
        return *this;
    }
    arg_t(arg_t&& rhs) noexcept : ptr(rhs.ptr) {
        rhs.ptr = nullptr;
    }
    arg_t& operator=(arg_t&& rhs) noexcept {
        this->~arg_t();
        ptr = rhs.ptr;
        rhs.ptr = nullptr;
        return *this;
    }
    bool operator==(const arg_t& rhs) const noexcept {
        return compare_dispatcher::dispatch(*ptr, *rhs.ptr, compare_visitor());
    }
public:
    string to_string() const noexcept {
        return ptr->to_string();
    }
#ifdef PY_EXT
    double to_double(const boost::python::dict& dict) const {
        return ptr->to_double(dict);
    }
#endif
    double to_double(const std::unordered_map<string, double>& dict) const noexcept {
        return ptr->to_double(dict);
    }
    arg_base* const get_ptr() const noexcept {
        return ptr;
    }
    ~arg_t() noexcept {
        delete ptr;
    }
};

} // namespace math

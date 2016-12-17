#include <string>
#include <cmath>
#include <boost/python/extract.hpp>
#include <boost/python/list.hpp>
#include "math_utils.hpp"
namespace py = boost::python;

template <typename T>
class vector {
    T v[3];
public:
    enum type { rec, cyl, sph };
    vector(double a, double b, double c) noexcept : v { a, b, c } {}
    explicit vector(py::list list, int t) noexcept : vector(list, type(t)) {}
    explicit vector(py::list list, type t = rec) noexcept {
        switch (t) {
            case 0:
                v[0] = py::extract<T>(list[0]);
                v[1] = py::extract<T>(list[1]);
                v[2] = py::extract<T>(list[2]);
                break;
            case 1:
                v[0] = py::extract<T>(list[0])*std::cos(py::extract<T>(list[1]));
                v[1] = py::extract<T>(list[0])*std::sin(py::extract<T>(list[1]));
                v[2] = py::extract<T>(list[2]);
                break;
            case 2:
                v[0] = py::extract<T>(list[0])*std::sin(py::extract<T>(list[1]))*std::cos(py::extract<T>(list[2]));
                v[1] = py::extract<T>(list[0])*std::sin(py::extract<T>(list[1]))*std::sin(py::extract<T>(list[2]));
                v[2] = py::extract<T>(list[0])*std::cos(py::extract<T>(list[2]));
        }
    }
    vector operator+(const vector& rhs) const noexcept {
        return{ v[0] + rhs[0], v[1] + rhs[1], v[2] + rhs[2] };
    }
    vector operator-(const vector& rhs) const noexcept {
        return{ v[0] - rhs[0], v[1] - rhs[1], v[2] - rhs[2] };
    }
    vector operator-() const noexcept {
        return{ -v[0], -v[1], -v[2] };
    }
    vector mul(double a) const noexcept {
        return{ v[0] * a, v[1] * a, v[2] * a };
    }
    auto inner(const vector& rhs) const noexcept {
        return v[0] * rhs[0] + v[1] * rhs[1] + v[2] * rhs[2];
    }
    vector cross(const vector& rhs) const noexcept {
        return{ v[1] * rhs[2] - v[2] * rhs[1],
                 v[2] * rhs[0] - v[0] * rhs[2],
                 v[0] * rhs[1] - v[1] * rhs[0] };
    }
    vector operator/(double a) const noexcept {
        return{ v[0] / a, v[1] / a, v[2] / a };
    }
    T& operator[](unsigned i) noexcept {
        return v[i];
    }
    T operator[](unsigned i) const noexcept {
        return v[i];
    }
    double mag() const noexcept {
        return hypot(hypot(v[0], v[1]), v[2]);
        //return hypot(v[0],v[1],v[2]);
    }
    double angle(const vector& other) const noexcept {
        return acos(inner(other) / mag() / other.mag());
    }
    vector unit() const noexcept {
        return *this / mag();
    }
    double proj(const vector& other) const noexcept {
        return inner(other.unit());
    }
    std::string str() const noexcept {
        return "[" + std::to_string(v[0]) + ", " +
            std::to_string(v[1]) + ", " +
            std::to_string(v[2]) + "]";
    }
};

bool operator==(const vector<double>& a, const vector<double>& b) noexcept {
    return double_equal(a[0], b[0]) && double_equal(a[1], b[1]) && double_equal(a[2], b[2]);
}

bool operator!=(const vector<double>& a, const vector<double>& b) noexcept {
    return !(a == b);
}

#include "vector.hpp"
#include "math_utils.hpp"

template <typename T>
bool is_coplannar(const vector<T>&, const vector<T>&, const vector<T>&);

template <>
bool is_coplannar<double>(const vector<double>& a, const vector<double>& b, const vector<double>& c) {
    // (a X b) => shared normal vector
    // c must be perpendicular to normal to be coplannar
    return double_equal_abs_only(a.cross(b).inner(c), 0);
}

template <typename T>
double distance_between_lines(const vector<T>& ab, const vector<T>& a, const vector<T>& bb, const vector<T>& b) {
    vector<T> base_diff = ab - bb;
    vector<T> normal = a.cross(b);
    if (normal != vector<T>{0,0,0})
        return fabs(base_diff.proj(normal));
    else
        return sqrt(base_diff.inner(base_diff) - pow(base_diff.proj(a /*or b*/), 2));
}

template <typename T>
bool check_projection_intersect(const vector<T>& ab, const vector<T>& a, const vector<T>& bb, const vector<T>& b) {
    //       /
    //      /
    //---------------
    //    /
    // As seen above, if two line intersects, one line
    // must pass through the other, lower to higher.
    // Connect both ends correspondingly of the vectors
    vector<T> base_diff = ab - bb;
    vector<T> end_diff = base_diff + a - b; // ab+a-(bb+b) => ab-bb+a-b
    auto det = a.cross(base_diff).inner(a.cross(end_diff));
    if (double_equal_abs_only(det, 0)) {
        if (is_on_segment(ab, a, bb) || is_on_segment (ab, a, bb + b))
            return true;
        else
            return false;
    }
    else if (det > 0)
        return false;
    else // det < 0
        return true;
}

template <typename T>
bool is_on_line(const vector<T>& base, const vector<T>& dir, const vector<T>& point) {
    return is_collinear(point - base, dir);
}

template <typename T>
bool is_on_segment(const vector<T>& base, const vector<T>& dir, const vector<T>& point) {
    auto temp = point - base;
    return is_collinear(temp, dir) && temp.inner(dir) >= 0 && temp.mag() <= dir.mag();
}

template <typename T>
bool is_perpendicular(const vector<T>&, const vector<T>&);

template<>
bool is_perpendicular<double>(const vector<double>& a, const vector<double>& b) {
    return double_equal(a.inner(b), 0);
}

template <typename T>
bool is_collinear(const vector<T>&, const vector<T>&);

template <>
bool is_collinear<double>(const vector<double>& a, const vector<double>& b) {
    auto temp = a.cross(b);
    return double_equal(temp[0], 0) && double_equal(temp[1], 0) && double_equal(temp[2], 0);
}

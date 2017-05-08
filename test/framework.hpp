#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP
#include <iostream>
#include <utility>
#include <chrono>

class TestFailed {
public:
    TestFailed() {
        std::cerr << "TEST FAILED\n";
    }
    template <class... Args>
    TestFailed(Args&&... args) {
        std::cerr << "TEST FAILED: ";
        print_variadic(std::forward<Args>(args)...);
        std::cerr << "\n";
    }
private:
    template <class T, class... Args>
    void print_variadic(T t, Args&&... args) {
        std::cerr << t;
        print_variadic(std::forward<Args>(args)...);
    }
    void print_variadic() noexcept {}
};

struct ScopedTimer {
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    ScopedTimer(const char* str) {
        std::cout << str << "\n";
        start = std::chrono::high_resolution_clock::now();
    }
    ~ScopedTimer() {
        std::cout << (double) std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now() - start).count() << "\n";
    }
};

template <typename T, typename U, class... Args>
inline void EXPECT_EQ(const T& a, const U& b, Args&&... args) {
    if (a == b)
        ;
    else
        throw TestFailed(__FUNCTION__, " ", a, " == ", b, std::forward<Args>(args)...);
}

template <typename T, typename U, class... Args>
inline void EXPECT_NEQ(const T& a, const U& b, Args&&... args) {
    if (a != b)
        ;
    else
        throw TestFailed(__FUNCTION__, " ", a, " == ", b, std::forward<Args>(args)...);
}

template <typename T, typename U, class... Args>
inline void EXPECT_TRUE(const T& a, Args&&... args) {
    if (a)
        ;
    else
        throw TestFailed(__FUNCTION__, " ", a, std::forward<Args>(args)...);
}

template <typename T, typename U, class... Args>
inline void EXPECT_FALSE(const T& a, Args&&... args) {
    if (a)
        throw TestFailed(__FUNCTION__, " ", a, std::forward<Args>(args)...);
}
#endif

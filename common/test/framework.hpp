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
 */

#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP
#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>
#include <chrono>

struct make_location {
    const char* func;
    int lineno;
    std::string operator()() const {
        std::ostringstream ss;
        ss << "at " << func << ": " << lineno;
        return ss.str();
    }
};

struct read_line {
    const char* file;
    int lineno;
    std::string operator()() const {
        std::string ret;
        std::ifstream f(file);
        for (int lineno = this->lineno; lineno != 1; --lineno)
            f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::getline(f, ret);
        return ret;
    }
};

class TestFailed {
public:
    TestFailed() {
        std::cerr << "TEST FAILED\n";
    }
    template <typename... Args>
    TestFailed(Args&&... args) {
        std::cerr << "TEST FAILED: ";
        print_variadic(std::forward<Args&&>(args)...);
        std::cerr << "\n";
    }
private:
    template <typename T, typename... Args>
    void print_variadic(T t, Args&&... args) {
        std::cerr << t;
        print_variadic(std::forward<Args&&>(args)...);
    }
    template <typename... Args>
    void print_variadic(const make_location& loc, Args&&... args) {
        std::cerr << loc();
        print_variadic(std::forward<Args&&>(args)...);
    }
    template <typename... Args>
    void print_variadic(const read_line& l, Args&&... args) {
        std::cerr << l();
        print_variadic(std::forward<Args&&>(args)...);
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

template <typename T, typename U, typename... Args>
inline void EXPECT_EQ(const T& a, const U& b, Args&&... args) {
    if (a == b)
        ;
    else
        throw TestFailed(__FUNCTION__, " ", a, " != ", b, " ", std::forward<Args>(args)...);
}

template <typename T, typename U, typename... Args>
inline void EXPECT_NEQ(const T& a, const U& b, Args&&... args) {
    if (a != b)
        ;
    else
        throw TestFailed(__FUNCTION__, " ", a, " == ", b, " ", std::forward<Args>(args)...);
}

template <typename T, typename... Args>
inline void EXPECT_TRUE(const T& a, Args&&... args) {
    if (a)
        ;
    else
        throw TestFailed(__FUNCTION__, " ", std::forward<Args>(args)...);
}

template <typename T, typename... Args>
inline void EXPECT_FALSE(const T& a, Args&&... args) {
    if (a)
        throw TestFailed(__FUNCTION__, " ", std::forward<Args>(args)...);
}
#endif
